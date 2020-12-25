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

#include "image/image.hh"
#include "image/worker.hh"
#include "logger/logger.hh"
#include "nbt/region.hh"
#include "utils/threaded_worker.hh"

namespace pixel_terrain::image {
    void image_generator::queue(std::shared_ptr<region_container> item) {
        logger::L(logger::DEBUG, "trying to queue %s\n",
                  item->out_file_.filename().string().c_str());

        thread_pool_->queue_job(move(item));
    }

    void image_generator::start() {
        logger::L(logger::DEBUG, "starting worker thread(s) ...\n");

        thread_pool_->start();
    }

    void image_generator::finish() { thread_pool_->finish(); }
} // namespace pixel_terrain::image
