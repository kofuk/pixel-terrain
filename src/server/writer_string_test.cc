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
