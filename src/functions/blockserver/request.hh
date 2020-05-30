#ifndef REQUEST_HH
#define REQUEST_HH

#include <string>
#include <unordered_map>

namespace mcmap::server {
    using namespace std;

    template <typename clazz> class request {
        clazz *reader;

        char int_buf[2048];
        size_t n_in_buf = 0;

        string method;
        string protocol;
        string version;

        unordered_map<string, string> fields;

        bool parse_sig(string const &line) {
            size_t start = 0;
            size_t end;
            for (end = start; end < line.size() && isalpha(line[end]); ++end) {
            }
            if (end == line.size() || line[end] != ' ') return false;
            method = line.substr(start, end);

            start = end + 1;

            for (end = start; end < line.size() && line[end] != '/'; ++end) {
            }
            if (end == line.size() || line[end] != '/') return false;
            protocol = line.substr(start, end - start);

            start = end + 1;

            for (end = start;
                 end < line.size() && (isdigit(line[end]) || line[end] == '.');
                 ++end) {
            }
            if (end != line.size()) return false;
            version = line.substr(start, end - start);

            return true;
        }

        string const read_request_line(bool *ok) {
            size_t end = 0;
            bool has_cr = false;
        read_until_cr:
            for (; end < n_in_buf; ++end) {
                if (int_buf[end] == '\r') {
                    has_cr = true;
                    break;
                }
            }
            if (!has_cr || end + 1 == n_in_buf) {
                if (n_in_buf != 2048) {
                    ssize_t n_read =
                        reader->fill_buffer(int_buf, 2048, n_in_buf);
                    if (n_read < 0) {
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
            if (end >= 2048) {
                *ok = false;
                return "";
            }
            if (int_buf[++end] != '\n') {
                *ok = false;
                return "";
            }
            string result(int_buf, int_buf + end - 1);

            move(int_buf + end + 1, int_buf + 2048, int_buf);
            n_in_buf -= end + 1;

            *ok = true;

            return result;
        }

    public:
        request(clazz *reader) : reader(reader) {}
        ~request() { delete reader; }

        bool parse_all() {
            bool ok;
            string line = read_request_line(&ok);
            if (!ok || !parse_sig(line)) {
                return false;
            }

            line = read_request_line(&ok);
            for (; ok && line.size(); line = read_request_line(&ok)) {
                size_t pos_colon = line.find(':');
                string key = line.substr(0, pos_colon);
                if (key.size() == 0) return false;

                if (pos_colon + 1 < line.size() && line[pos_colon + 1] == ' ') {
                    ++pos_colon;
                }
                string val = line.substr(pos_colon + 1);
                fields[key] = val;

                /* should be replaced by better way to avoid attack */
                if (fields.size() > 5) {
                    return false;
                }
            }

            return true;
        }

        string const get_method() const { return method; }

        string const get_protocol() const { return protocol; }

        string const get_version() const { return version; }

        size_t get_field_count() const { return fields.size(); }

        string const get_request_field(string const &key) {
            return fields[key];
        }
    };
} // namespace mcmap::server

#endif
