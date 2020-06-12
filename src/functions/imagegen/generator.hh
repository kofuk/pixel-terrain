#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "worker.hh"

namespace pixel_terrain::generator {
    /* generates 256x256 image */
    void generate_256(shared_ptr<QueuedItem> item);
} // namespace pixel_terrain::generator

#endif
