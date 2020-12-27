// SPDX-License-Identifier: MIT

/* Logger, that can be used from different thread safely. */

#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>

#ifdef OS_LINUX
#include <unistd.h>
#endif

#include "logger.hh"

namespace pixel_terrain::logger {
    namespace {
        std::mutex m;

        struct statistics {
            std::size_t generated;
            std::size_t reused;
        };

        std::unordered_map<std::string, statistics> stats;

#ifdef OS_LINUX
        bool tty_initialized;
        bool is_tty;

        inline void check_tty() {
            if (!tty_initialized) {
                tty_initialized = true;
                is_tty = ::isatty(STDERR_FILENO);
            }
        }
#endif
    } // namespace

    unsigned int log_level;

    void L(unsigned int log_level, char const *fmt, ...) {
        if (logger::log_level < log_level) return;

        std::va_list ap;
        va_start(ap, fmt);

        std::unique_lock<std::mutex> lock(m);

#ifdef OS_LINUX
        check_tty();

        switch (log_level) {
        case logger::INFO:
            if (is_tty)
                std::fputs("\e[1mPixelTerrain-\e[1;32mINFO\e[0m: ", stderr);
            else
                std::fputs("PixelTerrain-INFO: ", stderr);
            break;
        case logger::DEBUG:
            if (is_tty)
                std::fputs("\e[1mPixelTerrain-\e[1;33mDEBUG\e[0m: ", stderr);
            else
                std::fputs("PixelTerrain-DEBUG: ", stderr);
            break;
        case logger::ERROR:
            if (is_tty)
                std::fputs("\e[1mPixelTerrain-\e[1;31mERROR\e[0m: ", stderr);
            else
                std::fputs("PixelTerrain-ERROR: ", stderr);
            break;
        }
#else /* not OS_LINUX */
        switch (log_level) {
        case logger::INFO:
            std::fputs("PixelTerrain-INFO: ", stderr);
            break;
        case logger::DEBUG:
            std::fputs("PixelTerrain-DEBUG: ", stderr);
            break;
        case logger::ERROR:
            std::fputs("PixelTerrain-ERROR: ", stderr);
            break;
        }
#endif

        std::vfprintf(stderr, fmt, ap);

        va_end(ap);
    }

    void record_stat(bool regenerated, std::string const &label) {
        std::unique_lock<std::mutex> lock(m);

        if (regenerated)
            ++stats[label].generated;
        else
            ++stats[label].reused;
    }

    void show_stat() {
        L(INFO, "STATISTICS\n");
        for (auto itr = stats.begin(), E = stats.end(); itr != E; ++itr) {
            statistics s = itr->second;
            L(INFO, "  %s\n",
              itr->first.empty() ? "<no label>" : itr->first.c_str());
            L(INFO, "   |- Chunks generated: %zu\n", s.generated);
            L(INFO, "   |- Chunks reused:    %zu\n", s.reused);
            L(INFO, "   |- %% reused:         %zu\n",
              (s.reused * 100) / (s.generated + s.reused));
        }
    }

    namespace {
        std::size_t progress_max;
        std::size_t progress_current;
        unsigned int old_progress = 101;

        void progress_print() {
            unsigned int progress;
            if (progress_max == 0)
                progress = 0;
            else if (progress_max <= progress_current)
                progress = 100;
            else
                progress = (progress_current * 100) / progress_max;
            if (old_progress != progress) {
#ifdef OS_LINUX
                check_tty();

                if (is_tty && log_level == 0)
                    std::fprintf(stderr, "Generating...    %u%%\r", progress);
                else
                    std::fprintf(stderr, "Generating...    %u%%\n", progress);

#else
                std::fprintf(stderr, "Generating...    %u%%\n", progress);
#endif
            }

            old_progress = progress;
        }
    } // namespace

    void progress_bar_increase_total(int n) {
        std::unique_lock<std::mutex> lock(m);
        progress_max += n;
        progress_print();
    }
    void progress_bar_process_one() {
        std::unique_lock<std::mutex> lock(m);
        ++progress_current;
        progress_print();
    }
} // namespace pixel_terrain::logger
