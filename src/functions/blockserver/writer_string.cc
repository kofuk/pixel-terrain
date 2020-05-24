#include "writer_string.hh"

namespace mcmap::server {
    void writer_string::write_data(string const &data) {
        this->data.insert(this->data.end(), data.begin(), data.end());
    }

    void writer_string::write_data(int const data) {
        write_data(to_string(data));
    }

    writer_string::operator string() const {
        return string(data.data(), data.size());
    }
} // namespace mcmap::server
