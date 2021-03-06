// SPDX-License-Identifier: MIT

#include <cstdint>
#include <string>

#include <unistd.h>

#include "server/writer_unix.hh"

namespace pixel_terrain::server {
    writer_unix::writer_unix(int fd) : fd(fd) {}

    writer_unix::~writer_unix() { ::write(fd, buf.data(), off); }

    void writer_unix::write_data(std::string const &data) {
        for (char const c : data) {
            if (off >= buf_size) {
                ::write(fd, buf.data(), buf_size);
                off = 0;
            }
            buf[off++] = c;
        }
    }

    void writer_unix::write_data(int const num) {
        write_data(std::to_string(num));
    }

    auto writer_unix::get_current_buffer() -> std::uint8_t const * {
        return buf.data();
    }

    auto writer_unix::get_current_offset() const -> std::size_t { return off; }
} // namespace pixel_terrain::server
