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

#ifndef WRITER_UNIX_HH
#define WRITER_UNIX_HH

#include <string>

#include "server/writer.hh"

namespace pixel_terrain::server {
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
} // namespace pixel_terrain::server

#endif
