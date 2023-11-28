/* Copyright (C) Steve Rabin, 2000. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Steve Rabin, 2000"
 */

#pragma once
#include <string_view>

struct Measurement {
    float ave = 0;
    float min = 0;
    float max = 0;
};

void ProfileInit() noexcept;
void ProfileBegin(std::string_view name) noexcept;
void ProfileEnd(std::string_view name) noexcept;
void ProfileDumpOutputToBuffer() noexcept;
void StoreProfileInHistory(std::string_view name, float percent ) noexcept;
Measurement GetProfileFromHistory(std::string_view name) noexcept;
void ProfileDraw() noexcept;