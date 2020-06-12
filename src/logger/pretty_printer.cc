/* Text-based progress bar */

#include <iomanip>
#include <iostream>
#include <mutex>

#include <sys/ioctl.h>
#include <unistd.h>

#include "pretty_printer.hh"

using namespace std;

namespace pixel_terrain::pretty_printer {
    namespace {
        int total_count;
        int current_count;
        int term_width;

        mutex printer_mutex;

        void update_progress_bar() {
            if (term_width < 4 || !total_count) return;

            int percent = current_count * 100 / total_count;

            cerr << "\r";
            cerr << setw(3) << percent << '%';
            if (term_width - 8 <= 0) {
                return;
            }
            int n_white =
                (term_width - 8) * (static_cast<float>(percent) / 100);
            cerr << " [";
            for (int i = 0; i < n_white; ++i) {
                cerr << '=';
            }
            for (int i = n_white; i < term_width - 8; ++i) {
                cerr << ' ';
            }
            cerr << "] ";
            cerr << flush;
        }
    } // namespace

    void set_total(int total) {
        total_count = total;

        if (isatty(STDERR_FILENO)) {
            struct winsize w;
            ioctl(2, TIOCGWINSZ, &w);
            term_width = w.ws_col;
        }
    }

    void increment_progress_bar() {
        unique_lock<mutex> lock(printer_mutex);
        ++current_count;
        update_progress_bar();
    }
} // namespace pixel_terrain::pretty_printer
