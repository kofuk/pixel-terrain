// SPDX-License-Identifier: MIT

#ifndef WORKER_HH
#define WORKER_HH

#include <mutex>
#include <utility>

#include "nbt/region.hh"

namespace pixel_terrain::image {
    struct region_container {
        anvil::region *region;
        std::filesystem::path out_file_;

        region_container(anvil::region *region, std::filesystem::path const &out_file);
        ~region_container();
    };

    extern std::filesystem::path option_out_dir;
    extern int option_jobs;
    extern bool option_nether;
    extern bool option_generate_range;
    extern std::filesystem::path option_cache_dir;

    region_container *fetch_item();
    void queue_item(std::shared_ptr<region_container> item);
    void start_worker();
    void finish_worker();
} // namespace pixel_terrain::image

#endif
