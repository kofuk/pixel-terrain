// SPDX-License-Identifier: MIT

#ifndef IMAGE_HH
#define IMAGE_HH

#include <filesystem>

#include "image/containers.hh"
#include "image/worker.hh"
#include "nbt/chunk.hh"
#include "nbt/region.hh"
#include "utils/threaded_worker.hh"

namespace pixel_terrain::image {
    class image_generator {
        image::worker *worker_;
        threaded_worker<std::shared_ptr<region_container>> *thread_pool_;
        region_container *fetch();

    public:
        image_generator(options opt) {
            worker_ = new image::worker(opt);
            thread_pool_ =
                new threaded_worker<std::shared_ptr<region_container>>(
                    opt.n_jobs, [this](std::shared_ptr<region_container> item) {
                        this->worker_->generate_region(item);
                    });
        }

        ~image_generator() {
            delete thread_pool_;
            delete worker_;
        }

        void start();
        void queue(std::shared_ptr<region_container> item);
        void finish();
    };

    void init_block_list();
} // namespace pixel_terrain::image

#endif
