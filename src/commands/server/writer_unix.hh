#ifndef WRITER_UNIX_HH
#define WRITER_UNIX_HH

#include <string>

#include "writer.hh"

namespace pixel_terrain::commands::server {
    class writer_unix : public writer {
        static constexpr std::size_t buf_size = 2048;
        char buf[buf_size];
        std::size_t off = 0;
        int fd;

        writer_unix(writer_unix const &) = delete;
        writer_unix &operator=(writer_unix const &) = delete;

    public:
        writer_unix(int fd);
        ~writer_unix();

        void write_data(std::string const &data);
        void write_data(int const data);

        char const *get_current_buffer();
        std::size_t get_current_offset();
    };
} // namespace pixel_terrain::commands::server

#endif
