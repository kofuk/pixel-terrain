// SPDX-License-Identifier: MIT

#ifndef PATH_HACK_HH
#define PATH_HACK_HH
#include <cstdio>
#include <cstring>
#include <string>

namespace pixel_terrain {
#ifdef _WIN32
    using path_string = std::wstring;

    using namespace std::literals;
#define PATH_STR_LITERAL(str) L##str
#define FOPEN(name, mode) _wfopen((name), L##mode)
#else
    using path_string = std::string;

#define PATH_STR_LITERAL(str) str
#define FOPEN fopen
#endif
} // namespace pixel_terrain
#endif
