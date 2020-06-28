#ifndef WRITER_STRING_HH
#define WRITER_STRING_HH

#include <string>
#include <vector>

#include "writer.hh"

namespace pixel_terrain::commands::server {
    class writer_string : public writer {
        std::vector<char> data;

    public:
        void write_data(std::string const &data);
        void write_data(int const data);
        operator std::string() const;
    };
} // namespace pixel_terrain::commands::server

#endif
