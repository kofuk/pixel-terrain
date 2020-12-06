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

#include <string>

#include <unistd.h>

#include "server/writer_unix.hh"

namespace pixel_terrain::server {
    writer_unix::writer_unix(int fd) : fd(fd) {}

    writer_unix::~writer_unix() { write(fd, buf, off); }

    void writer_unix::write_data(std::string const &data) {
        for (char const c : data) {
            if (off >= buf_size) {
                write(fd, buf, buf_size);
                off = 0;
            }
            buf[off++] = c;
        }
    }

    void writer_unix::write_data(int const num) {
        write_data(std::to_string(num));
    }

    char const *writer_unix::get_current_buffer() { return buf; }

    size_t writer_unix::get_current_offset() { return off; }
} // namespace pixel_terrain::server
