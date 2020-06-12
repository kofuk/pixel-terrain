#ifndef WRITER_STRING_HH
#define WRITER_STRING_HH

#include <string>
#include <vector>

#include "writer.hh"

namespace pixel_terrain::commands::server {
    using namespace std;

    class writer_string : public writer {
        vector<char> data;

    public:
        void write_data(string const &data);
        void write_data(int const data);
        operator string() const;
    };
} // namespace pixel_terrain::commands::server

#endif
