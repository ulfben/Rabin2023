#define _CRT_SECURE_NO_WARNINGS
#include "profile.h"
#include "custom_time.h"
#include <iostream>
#include <string>
#include <cassert> 
#include <format>
#include <array>
#include <algorithm>
struct Measurement {
    float ave = 0;
    float min = 0;
    float max = 0;
};
void ProfileDumpOutputToBuffer() noexcept;
void StoreProfileInHistory(std::string_view name, float percent) noexcept;
Measurement GetProfileFromHistory(std::string_view name) noexcept;

struct Sample {
    std::string szName{};               // Name of sample
    float fStartTime = 0.0f;            // The current open profile start time
    float fAccumulator = 0.0f;          // All samples this frame added together
    float fChildrenSampleTime = 0.0f;   // Time taken by all children
    unsigned int iProfileInstances = 0; // # of times ProfileBegin called
    unsigned int iNumParents = 0;       // Number of profile parents
    int iOpenProfiles = 0;              // # of times ProfileBegin w/o ProfileEnd
    bool bValid = false;                // Whether this data is valid 
};

struct SampleHistory {
    std::string szName{};    // Name of the sample
    float fAve = 0.0f;       // Average time per frame (percentage)
    float fMin = 0.0f;       // Minimum time per frame (percentage)
    float fMax = 0.0f;       // Maximum time per frame (percentage)
    bool bValid = false;     // Whether the data is valid
};

constexpr size_t PROFILE_COUNT = 50;

std::array<Sample, PROFILE_COUNT> samples{};
std::array<SampleHistory, PROFILE_COUNT> history{};
float g_startProfile = 0.0f;
float g_endProfile = 0.0f;
std::string textBox = "";

void Profile::Init() noexcept {
    g_startProfile = GetExactTime();
}


void Profile::Begin(std::string_view name) noexcept {
    auto result = std::ranges::find_if(samples, [name](auto& s) noexcept {
        return s.bValid && s.szName == name;
        });
    if (result != samples.end()) { // Found the sample      
        result->iOpenProfiles++;
        result->iProfileInstances++;
        result->fStartTime = GetExactTime();
        assert(result->iOpenProfiles == 1); // max 1 open at once
        return;
    }

    //TODO: potentially, extract this to a "createNewSample(name)" function.
    auto unused = std::ranges::find_if(samples, [](auto& s) noexcept {
        return !s.bValid;
        });
    if (unused == samples.end()) { //TODO: make samples vector and let it grow instead?
        assert(!"Exceeded Max Available Profile Samples");
        return;
    }
    unused->szName = name; //this should clearly be a Samples constructor
    unused->bValid = true;
    unused->iOpenProfiles = 1;
    unused->iProfileInstances = 1;
    unused->fAccumulator = 0.0f;
    unused->fStartTime = GetExactTime();
    unused->fChildrenSampleTime = 0.0f;
}

void Profile::End(std::string_view name) noexcept {
    auto sample = std::find_if(samples.begin(), samples.end(),
        [name](const Sample& s) {
            return s.bValid && s.szName == name;
        });

    if (sample == std::end(samples)) {
        return;
    }
    unsigned int inner = 0;
    int parent = -1;
    float fEndTime = GetExactTime();
    unsigned int numParents = 0;
    sample->iOpenProfiles--;

    // Count all parents and find the immediate parent
    while (samples[inner].bValid == true) {
        if (samples[inner].iOpenProfiles >
            0) { // Found a parent (any open profiles are parents)
            numParents++;
            if (parent < 0) { // Replace invalid parent (index)
                parent = inner;
            }
            else if (samples[inner].fStartTime >=
                samples[parent]
                .fStartTime) { // Replace with more immediate parent
                parent = inner;
            }
        }
        inner++;
    }

    // Remember the current number of parents of the sample
    sample->iNumParents = numParents;

    if (parent >= 0) { // Record this time in fChildrenSampleTime (add it in)
        samples[parent].fChildrenSampleTime +=
            fEndTime - sample->fStartTime;
    }
    sample->fAccumulator += fEndTime - sample->fStartTime;
}

std::string addIndentations(std::string_view name, size_t level) {
    std::string indents("\t", level);
    return indents.append(name);
}


void ProfileDumpOutputToBuffer() noexcept {
    unsigned int i = 0;
    g_endProfile = GetExactTime();
    textBox.clear();
    textBox.append("  Ave :   Min :   Max :   # : Profile Name\n");
    textBox.append("--------------------------------------------\n");

    while (i < PROFILE_COUNT && samples[i].bValid == true) {
        if (samples[i].iOpenProfiles < 0) {
            assert(!"End() called without a Begin()");
        }
        else if (samples[i].iOpenProfiles > 0) {
            assert(!"Begin() called without a End()");
        }
        float sampleTime = samples[i].fAccumulator - samples[i].fChildrenSampleTime;
        float percentTime = (sampleTime / (g_endProfile - g_startProfile)) * 100.0f;

        // Add new measurement into the history and get the ave, min, and max
        StoreProfileInHistory(samples[i].szName, percentTime);
        auto [aveTime, minTime, maxTime] = GetProfileFromHistory(samples[i].szName);

        std::string measures = std::format("{:3.1f}  : {:3.1f}  : {:3.1f}", aveTime, minTime, maxTime);
        std::string indentedName = addIndentations(samples[i].szName, samples[i].iNumParents);
        std::string line = std::format("{}  : {:>3} : {}\n", measures, samples[i].iProfileInstances, indentedName);
        textBox.append(line);
        i++;
    }

    { // Reset samples for next frame   
        for (i = 0; i < PROFILE_COUNT; i++) {
            samples[i].bValid = false;
        }
        g_startProfile = GetExactTime();
    }
}
void StoreProfileInHistory(std::string_view name, float percent) noexcept {
    unsigned int i = 0;
    float newRatio = 0.8f * GetElapsedTime();
    if (newRatio > 1.0f) {
        newRatio = 1.0f;
    }
    float oldRatio = 1.0f - newRatio;

    while (i < PROFILE_COUNT && history[i].bValid == true) {
        if (samples[i].szName == name) { // Found the sample
            history[i].fAve = (history[i].fAve * oldRatio) + (percent * newRatio);
            if (percent < history[i].fMin) {
                history[i].fMin = percent;
            }
            else {
                history[i].fMin =
                    (history[i].fMin * oldRatio) + (percent * newRatio);
            }

            if (history[i].fMin < 0.0f) {
                history[i].fMin = 0.0f;
            }

            if (percent > history[i].fMax) {
                history[i].fMax = percent;
            }
            else {
                history[i].fMax =
                    (history[i].fMax * oldRatio) + (percent * newRatio);
            }
            return;
        }
        i++;
    }

    if (i < PROFILE_COUNT) { // Add to history
        history[i].szName = name;
        history[i].bValid = true;
        history[i].fAve = history[i].fMin = history[i].fMax = percent;
    }
    else {
        assert(!"Exceeded Max Available Profile Samples!");
    }
}
Measurement GetProfileFromHistory(std::string_view name) noexcept {
    unsigned int i = 0;
    Measurement result;
    while (i < PROFILE_COUNT && history[i].bValid == true) {
        if (history[i].szName == name) { // Found the sample
            result.ave = history[i].fAve;
            result.min = history[i].fMin;
            result.max = history[i].fMax;
            return result;
        }
        i++;
    }
    return result;
}
void Profile::Draw() noexcept {
    ProfileDumpOutputToBuffer();
    if (!textBox.empty()) {
        std::cout << textBox << "\n";
    }
}