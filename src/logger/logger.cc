// SPDX-License-Identifier: MIT

/* Logger, that can be used from different thread safely. */

#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <mutex>

#include "logger.hh"

namespace pixel_terrain::logger {
    namespace {
        std::mutex m;

        std::size_t generated;
        std::size_t reused;
    } // namespace

    unsigned int log_level;

    void L(unsigned int log_level, char const *fmt, ...) {
        if (logger::log_level < log_level) return;

        std::va_list ap;
        va_start(ap, fmt);

        std::unique_lock<std::mutex> lock(m);
        std::vfprintf(stderr, fmt, ap);

        va_end(ap);
    }

    void record_stat(bool regenerated) {
        std::unique_lock<std::mutex> lock(m);

        if (regenerated)
            ++generated;
        else
            ++reused;
    }

    void show_stat() {
        if (log_level >= INFO) {
            std::cout << "Statistics:\n";
            std::cout << "  Chunks generated: " << generated << '\n';
            std::cout << "  Chunks reused:    " << reused << '\n';
            std::cout << "  % reused:         "
                      << (reused * 100) / (generated + reused) << '\n';
        }
    }
} // namespace pixel_terrain::logger
