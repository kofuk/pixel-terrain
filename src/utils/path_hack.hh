// SPDX-License-Identifier: MIT

#ifndef PATH_HACK_HH
#define PATH_HACK_HH
#include <cstdio>
#include <cstring>
#include <string>

namespace pixel_terrain {
#ifdef OS_WIN
    using path_string = std::wstring;
    using path_char = std::wchar_t;
    static inline auto to_path_string(int num) -> path_string{
        return std::to_wstring(num);
    }

    using namespace std::string_literals;
#define PATH_STR_LITERAL(str) L##str
#define FOPEN(name, mode) _wfopen((name), L##mode)
#elif defined(OS_LINUX)
    using path_string = std::string;
    using path_char = char;
    static inline auto to_path_string(int num) -> path_string {
        return std::to_string(num);
    }

#define PATH_STR_LITERAL(str) str
#define FOPEN std::fopen
#endif
} // namespace pixel_terrain
#endif
