// SPDX-License-Identifier: MIT

/* Logger, that can be used from different thread safely. */

#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <mutex>

#ifdef __unix__
#include <unistd.h>
#endif

#include "logger.hh"

namespace pixel_terrain::logger {
    namespace {
        std::mutex m;

        std::size_t generated;
        std::size_t reused;

#ifdef __unix__
        bool tty_initialized;
        bool is_tty;
#endif
    } // namespace

    unsigned int log_level;

    void L(unsigned int log_level, char const *fmt, ...) {
        if (logger::log_level < log_level) return;

        std::va_list ap;
        va_start(ap, fmt);

        std::unique_lock<std::mutex> lock(m);

#ifdef __unix__
        if (!tty_initialized) {
            tty_initialized = true;
            is_tty = ::isatty(STDOUT_FILENO);
        }
#endif

        switch (log_level) {
        case logger::INFO:
#ifdef __unix__
            if (is_tty)
                std::fputs("\e[1mPixelTerrain-\e[1;32mINFO\e[0m: ", stderr);
            else
                std::fputs("PixelTerrain-INFO: ", stderr);
#else
            std::fputs("PixelTerrain-INFO: ", stderr);
#endif
            break;
        case logger::DEBUG:
#ifdef __unix__
            if (is_tty)
                std::fputs("\e[1mPixelTerrain-\e[1;33mDEBUG\e[0m: ", stderr);
            else
                std::fputs("PixelTerrain-DEBUG: ", stderr);
#else
            std::fputs("PixelTerrain-DEBUG: ", stderr);
#endif
            break;
        case logger::ERROR:
#ifdef __unix__
            if (is_tty)
                std::fputs("\e[1mPixelTerrain-\e[1;31mERROR\e[0m: ", stderr);
            else
                std::fputs("PixelTerrain-ERROR: ", stderr);
#else
            std::fputs("PixelTerrain-ERROR: ", stderr);
#endif
            break;
        }

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
