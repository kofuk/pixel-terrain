// SPDX-License-Identifier: MIT

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
        int prev_percent = -1;

        std::mutex printer_mutex;

        void update_progress() {
            if (!total_count) return;

            int percent = current_count * 100 / total_count;
            if (prev_percent == percent) {
                return;
            }
            prev_percent = percent;

            std::cerr << "Generating image: " << percent << "%\r";
        }
    } // namespace

    void set_total(int total) {
        total_count = total;
        update_progress();
    }

    void increment_progress_bar() {
        std::unique_lock<std::mutex> lock(printer_mutex);
        ++current_count;
        update_progress();
    }

    void finish_progress_bar() {
        current_count = total_count;
        update_progress();
        std::cerr << '\n';
    }
} // namespace pixel_terrain::pretty_printer
