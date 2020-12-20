// SPDX-License-Identifier: MIT

#include <cstdio>

#include "utils/threaded_worker.hh"

volatile int buf[65536];

namespace {
    inline int max(int a, int b) { return a > b ? a : b; }

    void cb_fast(int N) { std::printf("%d\e[J\r", N); }

    void cb_slow(int N) {
        std::printf("%d\e[J\r", N);
        for (int i = 0; i < 65536; ++i)
            buf[i] = i * N;
    }

    void fast_test() {
        std::puts("Running fast_test...");
        pixel_terrain::threaded_worker<int> worker(
            max(1, (int)std::thread::hardware_concurrency() - 1), &cb_fast);
        worker.start();

        for (int i = 0; i < 10000000; ++i)
            worker.queue_job(i);

        worker.finish();
        std::puts("\r\e[JDone.");
    }

    void slow_consumer_test() {
        std::puts("Running slow_consumer_test");
        pixel_terrain::threaded_worker<int> worker(
            max(1, (int)std::thread::hardware_concurrency() - 1), &cb_slow);
        worker.start();

        for (int i = 0; i < 10000000; ++i)
            worker.queue_job(i);

        worker.finish();
        std::puts("\r\e[JDone.");
    }

    void slow_producer_test() {
        std::puts("Running slow_producer_test");
        pixel_terrain::threaded_worker<int> worker(
            max(1, (int)std::thread::hardware_concurrency() - 1), &cb_slow);
        worker.start();

        for (int i = 0; i < 10000000; ++i) {
            worker.queue_job(i);
            for (int j = 0; j < 65536; ++j) {
                buf[j] = i * j;
            }
        }

        worker.finish();
        std::puts("\r\e[JDone.");
    }

    void few_item_test() {
        std::puts("Running few_item_test");
        pixel_terrain::threaded_worker<int> worker(
            max(1, (int)std::thread::hardware_concurrency() - 1), &cb_slow);
        worker.start();

        for (int i = 0;
             i < max(1, (int)std::thread::hardware_concurrency() - 1); ++i) {
            worker.queue_job(i);
        }

        worker.finish();
        std::puts("\r\e[JDone.");
    }
} // namespace

int main(void) {
    fast_test();
    slow_consumer_test();
    slow_producer_test();
    few_item_test();
}
