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

#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "writer_string.hh"

using namespace pixel_terrain::server;

BOOST_AUTO_TEST_CASE(writer_string_alpha) {
    writer_string w;

    std::string initial = w;
    BOOST_TEST(initial == "");

    w.write_data("ABC");
    BOOST_TEST(static_cast<std::string>(w) == "ABC");
    BOOST_TEST(initial == "");

    w.write_data("DEF");
    BOOST_TEST(static_cast<std::string>(w) == "ABCDEF");
}

BOOST_AUTO_TEST_CASE(writer_string_num) {
    writer_string w;

    w.write_data(1);
    BOOST_TEST(static_cast<std::string>(w) == "1");

    w.write_data(100);
    BOOST_TEST(static_cast<std::string>(w) == "1100");
}

BOOST_AUTO_TEST_CASE(writer_string_mixed) {
    writer_string w;

    w.write_data(1);
    BOOST_TEST(static_cast<std::string>(w) == "1");

    w.write_data("ABC");
    BOOST_TEST(static_cast<std::string>(w) == "1ABC");
}
