// SPDX-License-Identifier: MIT

#ifndef LOGGER_HH
#define LOGGER_HH

#include <string>

namespace pixel_terrain::logger {
    extern unsigned int log_level;

    static constexpr int DEBUG = 3;
    static constexpr int INFO = 2;
    static constexpr int ERROR = 1;

    __attribute__((format(printf, 2, 3))) void L(unsigned int log_level,
                                                 char const *fmt, ...);

    /* void d(std::string message); */
    /* void e(std::string message); */
    /* void i(std::string message); */

    void record_stat(bool regenerated);
    void show_stat();
} // namespace pixel_terrain::logger

#endif
