// SPDX-License-Identifier: MIT

#include "server/request.hh"

namespace pixel_terrain::server {
    request::request(reader *r) : request_reader(r) {}

    auto request::parse_sig(std::string const &line) -> bool {
        std::size_t start = 0;
        std::size_t end;
        for (end = start;
             end < line.size() && static_cast<bool>(isalpha(line[end]));
             ++end) {
        }
        if (end == line.size() || line[end] != ' ') {
            return false;
        }
        method = line.substr(start, end);

        start = end + 1;

        for (end = start; end < line.size() && line[end] != '/'; ++end) {
        }
        if (end == line.size() || line[end] != '/') {
            return false;
        }
        protocol = line.substr(start, end - start);

        start = end + 1;

        for (end = start;
             end < line.size() &&
             (static_cast<bool>(isdigit(line[end])) || line[end] == '.');
             ++end) {
        }
        if (end != line.size()) {
            return false;
        }
        version = line.substr(start, end - start);

        return true;
    }

    auto request::read_request_line(bool *ok) -> std::string {
        std::size_t end = 0;
        bool has_cr = false;
    read_until_cr:
        for (; end < n_in_buf; ++end) {
            if (int_buf[end] == '\n') {
                *ok = false;
                return "";
            }
            if (int_buf[end] == '\r') {
                has_cr = true;
                break;
            }
        }
        if (!has_cr || end + 1 == n_in_buf) {
            if (n_in_buf != sizeof(int_buf)) {
                long int n_read = request_reader->fill_buffer(
                    int_buf, sizeof(int_buf), n_in_buf);
                if (n_read <= 0) {
                    *ok = false;
                    return "";
                }
                n_in_buf += n_read;
                if (!has_cr) {
                    goto read_until_cr;
                }
            } else {
                *ok = false;
                return "";
            }
        }
        if (end >= sizeof(int_buf)) {
            *ok = false;
            return "";
        }
        if (int_buf[++end] != '\n') {
            *ok = false;
            return "";
        }
        std::string result(int_buf, int_buf + end - 1);

        std::move(int_buf + end + 1, int_buf + sizeof(int_buf), int_buf);
        n_in_buf -= end + 1;

        *ok = true;

        return result;
    }

    auto request::parse_all() -> bool {
        bool ok;
        std::string line = read_request_line(&ok);
        if (!ok || !parse_sig(line)) {
            return false;
        }

        line = read_request_line(&ok);
        for (; ok && !line.empty(); line = read_request_line(&ok)) {
            std::size_t pos_colon = line.find(':');
            std::string key = line.substr(0, pos_colon);
            if (key.empty()) {
                return false;
            }

            if (pos_colon + 1 < line.size() && line[pos_colon + 1] == ' ') {
                ++pos_colon;
            }
            std::string val = line.substr(pos_colon + 1);
            fields[key] = val;

            /* should be replaced by better way to avoid attack */
            constexpr std::size_t max_fields = 5;
            if (fields.size() > max_fields) {
                return false;
            }
        }

        return true;
    }

    auto request::get_method() const noexcept -> std::string { return method; }

    auto request::get_protocol() const noexcept -> std::string {
        return protocol;
    }

    auto request::get_version() const noexcept -> std::string {
        return version;
    }

    auto request::get_field_count() const noexcept -> std::size_t {
        return fields.size();
    }

    auto request::get_request_field(std::string const &key) noexcept
        -> std::string {
        return fields[key];
    }

} // namespace pixel_terrain::server
