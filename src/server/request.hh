/*
 * Copyright (c) 2020 Koki Fukuda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef REQUEST_HH
#define REQUEST_HH

#include <string>
#include <unordered_map>

#include "reader.hh"

namespace pixel_terrain::server {
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
} // namespace pixel_terrain::server

#endif
