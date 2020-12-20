// SPDX-License-Identifier: MIT

#ifndef CONTAINERS_HH
#define CONTAINERS_HH

#include <filesystem>
#include <thread>

#include "nbt/chunk.hh"
#include "nbt/region.hh"
#include "utils/path_hack.hh"

namespace pixel_terrain::image {
    struct region_container {
        anvil::region *region;
        std::filesystem::path out_file_;

        region_container(anvil::region *region,
                         std::filesystem::path const &out_file)
            : region(region), out_file_(out_file) {}
        ~region_container() { delete region; }
    };

    struct options {
        std::filesystem::path out_dir;
        int n_jobs;
        bool nether;
        bool generate_range;
        std::filesystem::path cache_dir;

        options() {
            clear();
        }

        void clear() {
            out_dir = PATH_STR_LITERAL(".");
            n_jobs = std::thread::hardware_concurrency();
            nether = false;
            generate_range = false;
            cache_dir.clear();
        }
    };
} // namespace pixel_terrain::image

#endif
