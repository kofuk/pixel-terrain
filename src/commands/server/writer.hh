#ifndef WRITER_HH
#define WRITER_HH

#include <string>

using namespace std;

namespace pixel_terrain::commands::server {
    class writer {
    public:
        virtual ~writer() = default;
        virtual void write_data(string const &data) = 0;
        virtual void write_data(int const data) = 0;
    };
} // namespace pixel_terrain::commands::server

#endif
