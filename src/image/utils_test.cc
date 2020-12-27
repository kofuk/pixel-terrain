// SPDX-License-Identifier: MIT

#include <filesystem>

#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "image/utils.hh"
#include "utils/path_hack.hh"

using namespace pixel_terrain;

BOOST_AUTO_TEST_CASE(make_output_name_test) {
    std::filesystem::path path(PATH_STR_LITERAL("/foo/bar/foo.mca"));
    std::filesystem::path out_dir(PATH_STR_LITERAL("/foobar"));
    {
        auto [name, ok] = image::make_output_name_by_input(path, out_dir);
        BOOST_TEST(ok);
        BOOST_TEST(name == PATH_STR_LITERAL("/foobar/foo.png"));
    }

    {
        path = PATH_STR_LITERAL("/foo/bar.baz");
        auto [name, ok] = image::make_output_name_by_input(path, out_dir);
        BOOST_TEST(not ok);
    }
}

BOOST_AUTO_TEST_CASE(format_output_name_test) {
    BOOST_TEST(image::format_output_name("%X %Z", 1, 2) == "1 2");
    BOOST_TEST(image::format_output_name("%X %Z", 10, 200) == "10 200");
    BOOST_TEST(image::format_output_name("%", 10, 200) == "%");
    BOOST_TEST(image::format_output_name("%%%X", 10, 200) == "%10");
    BOOST_TEST(image::format_output_name("", 10, 200) == "");
    BOOST_TEST(image::format_output_name("%a", 10, 200) == "%a");
}
