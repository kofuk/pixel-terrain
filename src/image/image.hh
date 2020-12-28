// SPDX-License-Identifier: MIT

#ifndef IMAGE_HH
#define IMAGE_HH

#include <filesystem>

#include "image/containers.hh"
#include "image/worker.hh"
#include "logger/logger.hh"
#include "nbt/chunk.hh"
#include "nbt/region.hh"
#include "utils/path_hack.hh"
#include "utils/threaded_worker.hh"

namespace pixel_terrain::image {
    class image_generator {
        image::worker *worker_;
        threaded_worker<region_container *> *thread_pool_;
        region_container *fetch();

        void write_range_file(int start_x, int start_z, int end_x, int end_z,
                              options const &options);

    public:
        image_generator(options options) {
            worker_ = new image::worker;
            thread_pool_ = new threaded_worker<region_container *>(
                options.n_jobs(), [this](region_container *item) {
                    this->worker_->generate_region(item);
                    logger::progress_bar_process_one();
                    delete item;
                });
        }

        ~image_generator() {
            delete thread_pool_;
            delete worker_;
        }

        void start();
        void queue(region_container *item);
        void queue_region(std::filesystem::path const &region_file,
                          options const &options);
        void queue_all_in_dir(std::filesystem::path const &dir,
                              options const &options);
        void finish();
    };
} // namespace pixel_terrain::image

#endif
