/* Text-based progress bar */

#include <iomanip>
#include <iostream>
#include <mutex>

#ifndef _WIN32
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include "pretty_printer.hh"

namespace pixel_terrain::pretty_printer {
    namespace {
        int total_count;
        int current_count;
        int term_width;

        std::mutex printer_mutex;

        void update_progress_bar() {
            if (term_width < 4 || !total_count) return;

            int percent = current_count * 100 / total_count;

            std::cerr << "\r";
            std::cerr << std::setw(3) << percent << '%';
            if (term_width - 8 <= 0) {
                return;
            }
            int n_white =
                (term_width - 8) * (static_cast<float>(percent) / 100);
            std::cerr << " [";
            for (int i = 0; i < n_white; ++i) {
                std::cerr << '=';
            }
            for (int i = n_white; i < term_width - 8; ++i) {
                std::cerr << ' ';
            }
            std::cerr << "] ";
            std::cerr << std::flush;
        }
    } // namespace

    void set_total(int total) {
        total_count = total;

#ifdef _WIN32
        term_width = 40;
#else
        if (isatty(STDERR_FILENO)) {
            struct winsize w;
            ioctl(2, TIOCGWINSZ, &w);
            term_width = w.ws_col;
        }
#endif
    }

    void increment_progress_bar() {
        std::unique_lock<std::mutex> lock(printer_mutex);
        ++current_count;
        update_progress_bar();
    }
} // namespace pixel_terrain::pretty_printer
