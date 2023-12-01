# Rabin2023
A refactoring exercise in API Design &amp; Best Practices 2023, bringing an old C-code base into Modern C++.

**2023-11-24 Code Review Workshop:**
How can we improve the old legacy C-code, to match modern C++ best practices? 

* ~~Replace all const char* with std::string_view~~
* ~~Replace all char[] with std::string~~
* ~~Get rid of macros (prefer containers that know their own size).~~
* ~~“Get” functions are expected to return something. Get rid of the output parameters and use return the values instead. (structured binding!)~~
* Consider namespace for free functions.
* ~~C-style arrays should be std containers (std array or vector).~~
* Remove Hungarian notation.
* For loops to ranged.
* Most loops should be algorithms.
* ~~Remove void-parameters~~
* ~~Replace C-headers with C++ headers~~
  * ~~<assert.h>, <string.h> etc. “<.h>” indicates it's a C-header, with no namespaces~~.
  * ~~If you need C-functions, prefer the C++ headers; <cassert>, <cmath> etc~~. 
* ~~Replace C-functions with std library stuff (string, format, array etc):~~
  * ~~printf => std::format~~
  * ~~strcmp => operator==~~
  * ~~strcpy => operator=~~
* Uninitialized variables. declare only when you are ready to use them
* Reduce the length of functions (invert and extract!)
* Reduce depth of scopes (invert and extract!)
* Add noexcept to functions that can't throw
* Add constructors to structs instead of doing two-step initialization
* Consider removing "isValid" and tie validity to lifetime instead. (if the object exist, it is valid)
* ~~Maybe replace include guards with pragma once~~
* ???
