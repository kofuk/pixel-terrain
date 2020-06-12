#ifndef REQUEST_HH
#define REQUEST_HH

#include <string>
#include <unordered_map>

#include "reader.hh"

namespace pixel_terrain::commands::server {
    using namespace std;

    class request {
        reader *request_reader;

        char int_buf[2048];
        size_t n_in_buf = 0;

        string method;
        string protocol;
        string version;

        unordered_map<string, string> fields;

        bool parse_sig(string const &line);
        string const read_request_line(bool *ok);

    public:
        request(reader *r);

        bool parse_all();
        string const get_method() const noexcept;
        string const get_protocol() const noexcept;
        string const get_version() const noexcept;
        size_t get_field_count() const noexcept;
        string const get_request_field(string const &key) noexcept;
    };
} // namespace pixel_terrain::commands::server

#endif
