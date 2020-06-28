#include <iostream>

#include "writer_generic.hh"

namespace pixel_terrain::commands::server {
    writer_generic::writer_generic() {}
    writer_generic::~writer_generic() {}

    void writer_generic::write_data(std::string const &data) {
        std::cout << data;
    }

    void writer_generic::write_data(int const data) { std::cout << data; }
} // namespace pixel_terrain::commands::server
