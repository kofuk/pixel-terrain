#include <string>

#include <unistd.h>

#include "writer_unix.hh"

namespace pixel_terrain::commands::server {
    writer_unix::writer_unix(int fd) : fd(fd) {}

    writer_unix::~writer_unix() { write(fd, buf, off); }

    void writer_unix::write_data(string const &data) {
        for (char const c : data) {
            if (off >= buf_size) {
                write(fd, buf, buf_size);
                off = 0;
            }
            buf[off++] = c;
        }
    }

    void writer_unix::write_data(int const num) { write_data(to_string(num)); }

    char const *writer_unix::get_current_buffer() { return buf; }

    size_t writer_unix::get_current_offset() { return off; }
} // namespace pixel_terrain::commands::server
