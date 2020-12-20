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
#include "image/image.hh"
#include "logger/logger.hh"
#include "nbt/region.hh"
#include "utils/threaded_worker.hh"

namespace pixel_terrain::image {
    void worker::queue(std::shared_ptr<region_container> item) {
        logger::L(logger::DEBUG, "trying to queue %s\n",
                  item->out_file_.filename().string().c_str());

        worker_->queue_job(move(item));
    }

    void worker::start() {
        logger::L(logger::DEBUG, "starting worker thread(s) ...\n");

        worker_->start();
    }

    void worker::finish() { worker_->finish(); }
} // namespace pixel_terrain::image
