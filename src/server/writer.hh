// SPDX-License-Identifier: MIT

#ifndef WRITER_HH
#define WRITER_HH

#include <string>

namespace pixel_terrain::server {
    class writer {
    public:
        virtual ~writer() = default;
        virtual void write_data(std::string const &data) = 0;
        virtual void write_data(int const data) = 0;
    };
} // namespace pixel_terrain::server

#endif
