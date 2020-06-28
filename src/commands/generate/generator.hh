#ifndef GENERATOR_HH
#define GENERATOR_HH

#include <memory>

#include "worker.hh"

namespace pixel_terrain::commands::generate {
    /* generates 256x256 image */
    void generate_256(std::shared_ptr<queued_item> item);
} // namespace pixel_terrain::commands::generate

#endif
