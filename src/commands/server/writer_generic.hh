#ifndef WRITER_GENERIC_HH
#define WRITER_GENERIC_HH

#include <string>

#include "writer.hh"

namespace pixel_terrain::commands::server {
    class writer_generic : public writer {
    public:
        writer_generic();
        ~writer_generic();

        void write_data(std::string const &data);
        void write_data(int const data);
    };
} // namespace pixel_terrain::commands::server

#endif
