#include "writer_string.hh"

namespace pixel_terrain::commands::server {
    void writer_string::write_data(std::string const &data) {
        this->data.insert(this->data.end(), data.begin(), data.end());
    }

    void writer_string::write_data(int const data) {
        write_data(std::to_string(data));
    }

    writer_string::operator std::string() const {
        return std::string(data.data(), data.size());
    }
} // namespace pixel_terrain::commands::server
