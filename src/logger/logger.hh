// SPDX-License-Identifier: MIT

#ifndef LOGGER_HH
#define LOGGER_HH

#include <string>

namespace pixel_terrain::logger {
    extern unsigned int log_level;

    static constexpr int DEBUG = 3;
    static constexpr int INFO = 2;
    static constexpr int ERROR = 1;

    __attribute__((format(printf, 2, 3))) void print_log(unsigned int log_level,
                                                         char const *fmt, ...);

    void record_stat(bool regenerated, std::string const &label);
    void show_stat();

    void progress_bar_increase_total(int n);
    void progress_bar_process_one();
} // namespace pixel_terrain::logger

#define ILOG(fmt, ...)                                            \
    pixel_terrain::logger::print_log(pixel_terrain::logger::INFO, \
                                     fmt __VA_OPT__(, ) __VA_ARGS__)
#define DLOG(fmt, ...)                                              \
    pixel_terrain::logger::print_log(pixel_terrain::logger::DEBUG,  \
                                     "%s(%d): " fmt, __FILE__, \
                                     __LINE__ __VA_OPT__(, ) __VA_ARGS__)

#define ELOG(fmt, ...)                                              \
    pixel_terrain::logger::print_log(pixel_terrain::logger::ERROR,  \
                                     "%s(%d): " fmt, __FILE__, \
                                     __LINE__ __VA_OPT__(, ) __VA_ARGS__)
#endif
