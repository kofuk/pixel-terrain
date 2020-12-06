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
