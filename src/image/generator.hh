// SPDX-License-Identifier: MIT

#ifndef GENERATOR_HH
#define GENERATOR_HH

#include <filesystem>
#include <memory>

#include "image/worker.hh"

namespace pixel_terrain::image {
    /* generates 256x256 image */
    void generate_region(std::shared_ptr<region_container> item);
} // namespace pixel_terrain::image

#endif
