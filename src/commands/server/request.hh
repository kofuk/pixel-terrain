#ifndef REQUEST_HH
#define REQUEST_HH

#include <string>
#include <unordered_map>

#include "reader.hh"

namespace pixel_terrain::commands::server {
    class request {
        reader *request_reader;

        char int_buf[2048];
        std::size_t n_in_buf = 0;

        std::string method;
        std::string protocol;
        std::string version;

        std::unordered_map<std::string, std::string> fields;

        bool parse_sig(std::string const &line);
        std::string const read_request_line(bool *ok);

    public:
        request(reader *r);

        bool parse_all();
        std::string const get_method() const noexcept;
        std::string const get_protocol() const noexcept;
        std::string const get_version() const noexcept;
        std::size_t get_field_count() const noexcept;
        std::string const get_request_field(std::string const &key) noexcept;
    };
} // namespace pixel_terrain::commands::server

#endif
