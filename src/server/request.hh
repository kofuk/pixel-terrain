// SPDX-License-Identifier: MIT

#ifndef REQUEST_HH
#define REQUEST_HH

#include <string>
#include <unordered_map>

#include "server/reader.hh"

namespace pixel_terrain::server {
    class request {
        reader *request_reader;

        static constexpr std::size_t IO_BUF_SIZE = 2048;

        std::array<std::uint8_t, IO_BUF_SIZE> int_buf;
        std::size_t n_in_buf = 0;

        std::string method;
        std::string protocol;
        std::string version;

        std::unordered_map<std::string, std::string> fields;

        auto parse_sig(std::string const &line) -> bool;
        auto read_request_line(bool *ok) -> std::string;

    public:
        request(reader *r);

        auto parse_all() -> bool;
        auto get_method() const noexcept -> std::string;
        auto get_protocol() const noexcept -> std::string;
        auto get_version() const noexcept -> std::string;
        auto get_field_count() const noexcept -> ::size_t;
        auto get_request_field(std::string const &key) noexcept -> std::string;
    };
} // namespace pixel_terrain::server

#endif
