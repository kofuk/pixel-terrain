// SPDX-License-Identifier: MIT

#ifndef READER_HH
#define READER_HH

#include <cstddef>
#include <cstdint>

namespace pixel_terrain::server {
    class reader {
    public:
        virtual ~reader() = default;
        virtual auto fill_buffer(std::uint8_t *buf, std::size_t len,
                                 std::size_t off) -> long int = 0;
    };
} // namespace pixel_terrain::server

#endif
