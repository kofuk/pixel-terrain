// SPDX-License-Identifier: MIT

#ifndef IMAGE_HH
#define IMAGE_HH

#include <filesystem>

#include "image/containers.hh"
#include "image/generator.hh"
#include "nbt/chunk.hh"
#include "nbt/region.hh"
#include "utils/threaded_worker.hh"

namespace pixel_terrain::image {
    class worker {
        image::image_generator *generator_;
        threaded_worker<std::shared_ptr<region_container>> *worker_;
        region_container *fetch();

    public:
        worker(options opt) {
            generator_ = new image::image_generator(opt);
            worker_ = new threaded_worker<std::shared_ptr<region_container>>(
                opt.n_jobs, [this](std::shared_ptr<region_container> item) {
                    this->generator_->generate_region(item);
                });
        }

        ~worker() {
            delete worker_;
            delete generator_;
        }

        void start();
        void queue(std::shared_ptr<region_container> item);
        void finish();
    };

    void init_block_list();
} // namespace pixel_terrain::image

#endif
