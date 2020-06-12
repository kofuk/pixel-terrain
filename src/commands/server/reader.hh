#ifndef READER_HH
#define READER_HH

#include <cstddef>

namespace pixel_terrain::commands::server {
    class reader {
    public:
        virtual ~reader() = default;
        virtual long int fill_buffer(char *buf, size_t len, size_t off) = 0;
    };
} // namespace pixel_terrain::commands::server

#endif
