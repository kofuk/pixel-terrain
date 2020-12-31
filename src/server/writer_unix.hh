// SPDX-License-Identifier: MIT

#ifndef WRITER_UNIX_HH
#define WRITER_UNIX_HH

#include <array>
#include <cstdint>
#include <string>

#include "server/writer.hh"

namespace pixel_terrain::server {
    class writer_unix : public writer {
        static constexpr std::size_t buf_size = 2048;
        std::array<std::uint8_t, buf_size> buf;
        std::size_t off = 0;
        int fd;

    public:
        writer_unix(writer_unix const &) = delete;
        auto operator=(writer_unix const &) -> writer_unix & = delete;

        writer_unix(int fd);
        ~writer_unix() override;

        void write_data(std::string const &data) override;
        void write_data(int num) override;

        [[nodiscard]] auto get_current_buffer() -> std::uint8_t const *;
        [[nodiscard]] auto get_current_offset() const -> std::size_t;
    };
} // namespace pixel_terrain::server

#endif
