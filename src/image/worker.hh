/*
 * Copyright (c) 2020 Koki Fukuda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef WORKER_HH
#define WORKER_HH

#include <mutex>
#include <utility>

#include "../nbt/region.hh"

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
    void wait_for_worker();
    void finish_worker();
} // namespace pixel_terrain::image

#endif
