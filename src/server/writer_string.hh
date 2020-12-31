// SPDX-License-Identifier: MIT

#ifndef WRITER_STRING_HH
#define WRITER_STRING_HH

#include <string>
#include <vector>

#include "server/writer.hh"

namespace pixel_terrain::server {
    class writer_string : public writer {
        std::vector<char> data;

    public:
        void write_data(std::string const &data) override;
        void write_data(int data) override;
        operator std::string() const;
    };
} // namespace pixel_terrain::server

#endif
