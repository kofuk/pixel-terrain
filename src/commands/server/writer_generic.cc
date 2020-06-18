#include <iostream>

#include "writer_generic.hh"

namespace pixel_terrain::commands::server {
    writer_generic::writer_generic() {}
    writer_generic::~writer_generic() {}

    void writer_generic::write_data(string const &data) { cout << data; }

    void writer_generic::write_data(int const data) { cout << data; }
} // namespace pixel_terrain::commands::server
