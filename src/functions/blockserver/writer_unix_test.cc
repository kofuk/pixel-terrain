#include <cstring>

#include <fcntl.h>
#include <unistd.h>

#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "writer_unix.hh"

using namespace pixel_terrain;

BOOST_AUTO_TEST_CASE(writer_unix_normal) {
    int fd = open("/dev/null", O_WRONLY);
    if (fd < 0) {
        perror("writer_unix_test");
        BOOST_TEST_FAIL("Failed to open /dev/null");
    }

    server::writer_unix w(fd);
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

    server::writer_unix w(fd);

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
