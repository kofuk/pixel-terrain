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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <memory>
#include <stdexcept>

#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "nbt/utils.hh"

using namespace pixel_terrain::nbt::utils;

BOOST_AUTO_TEST_CASE(parse_region_file_name_test) {
    {
        auto [x, z] =
            parse_region_file_path(std::filesystem::path("r.1.2.mca"));
        BOOST_TEST(x == 1);
        BOOST_TEST(z == 2);
    }

    {
        auto [x, z] =
            parse_region_file_path(std::filesystem::path("r.-1.2.mca"));
        BOOST_TEST(x == -1);
        BOOST_TEST(z == 2);
    }

    {
        auto [x, z] =
            parse_region_file_path(std::filesystem::path("r.1.-2.mca"));
        BOOST_TEST(x == 1);
        BOOST_TEST(z == -2);
    }

    {
        auto [x, z] =
            parse_region_file_path(std::filesystem::path("r.-1.-2.mca"));
        BOOST_TEST(x == -1);
        BOOST_TEST(z == -2);
    }

    {
        BOOST_CHECK_THROW(
            parse_region_file_path(std::filesystem::path("r.1.2.mca.")),
            std::invalid_argument);
    }

    {
        BOOST_CHECK_THROW(
            parse_region_file_path(std::filesystem::path("r.1a.2.mca")),
            std::invalid_argument);
    }

    {
        BOOST_CHECK_THROW(
            parse_region_file_path(std::filesystem::path("r.1.2a.mca")),
            std::invalid_argument);
    }

    {
        BOOST_CHECK_THROW(
            parse_region_file_path(std::filesystem::path(".r.1.2.mca")),
            std::invalid_argument);
    }

    { BOOST_CHECK_THROW(parse_region_file_path("foo"), std::invalid_argument); }
}
