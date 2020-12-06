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

#include <cstring>

#include <fcntl.h>
#include <unistd.h>

#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "server/writer_unix.hh"

using namespace pixel_terrain::server;

BOOST_AUTO_TEST_CASE(writer_unix_normal) {
    int fd = open("/dev/null", O_WRONLY);
    if (fd < 0) {
        perror("writer_unix_test");
        BOOST_TEST_FAIL("Failed to open /dev/null");
    }

    writer_unix w(fd);
    w.write_data("ABC");
    BOOST_TEST(w.get_current_offset() == 3);
    BOOST_TEST(strncmp(w.get_current_buffer(), "ABC", 3) == 0);

    w.write_data(1);
    BOOST_TEST(w.get_current_offset() == 4);
    BOOST_TEST(w.get_current_buffer()[3] == '1');

    w.write_data(10);
    BOOST_TEST(w.get_current_offset() == 6);
    BOOST_TEST(w.get_current_buffer()[4] == '1');
    BOOST_TEST(w.get_current_buffer()[5] == '0');

    close(fd);
}

BOOST_AUTO_TEST_CASE(writer_unix_exceed) {
    int fd = open("/dev/null", O_WRONLY);
    if (fd < 0) {
        perror("writer_unix_test");
        BOOST_TEST_FAIL("Failed to open /dev/null");
    }

    writer_unix w(fd);

    for (int i = 0; i < 2048; ++i) {
        w.write_data(0);
    }

    BOOST_TEST(w.get_current_offset() == 2048);

    w.write_data(1);
    BOOST_TEST(w.get_current_offset() == 1);
    BOOST_TEST(w.get_current_buffer()[0] == '1');

    for (int i = 0; i < 2046; ++i) {
        w.write_data(1);
    }
    BOOST_TEST(w.get_current_offset() == 2047);

    w.write_data("AB");
    BOOST_TEST(w.get_current_offset() == 1);
    BOOST_TEST(w.get_current_buffer()[0] == 'B');

    for (int i = 0; i < 2046; ++i) {
        w.write_data(2);
    }
    BOOST_TEST(w.get_current_offset() == 2047);

    w.write_data(10);
    BOOST_TEST(w.get_current_offset() == 1);
    BOOST_TEST(w.get_current_buffer()[0] == '0');

    close(fd);
}
