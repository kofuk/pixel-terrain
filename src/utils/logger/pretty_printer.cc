/*
 * Copyright (c) 2020 Koki Fukuda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
