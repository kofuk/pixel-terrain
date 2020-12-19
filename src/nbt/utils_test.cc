// SPDX-License-Identifier: MIT

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
