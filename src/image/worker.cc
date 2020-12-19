// SPDX-License-Identifier: MIT

/* Helper to use multi-thread efficiently. */

#include <algorithm>
#include <condition_variable>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iterator>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "image/generator.hh"
#include "image/worker.hh"
#include "logger/logger.hh"
#include "nbt/region.hh"
#include "utils/threaded_worker.hh"

namespace pixel_terrain::image {
    region_container::region_container(anvil::region *region,
                                       std::filesystem::path const &out_file)
        : region(region), out_file_(out_file) {}

    region_container::~region_container() { delete region; }

    namespace {
        threaded_worker<std::shared_ptr<region_container>> *worker;
    } // namespace

    std::filesystem::path option_out_dir;
    int option_jobs;
    bool option_nether;
    bool option_generate_progress;
    bool option_generate_range;
    std::filesystem::path option_cache_dir;

    void queue_item(std::shared_ptr<region_container> item) {
        logger::L(logger::DEBUG, "trying to queue %s\n",
                  item->out_file_.filename().string().c_str());

        worker->queue_job(move(item));
    }

    void start_worker() {
        logger::L(logger::DEBUG, "starting worker thread(s) ...\n");

        worker = new threaded_worker<std::shared_ptr<region_container>>(
            option_jobs, &generate_region);
        worker->start();
    }

    void wait_for_worker() {
        worker->wait();
        delete worker;
    }

    void finish_worker() { worker->finish(); }
} // namespace pixel_terrain::image
