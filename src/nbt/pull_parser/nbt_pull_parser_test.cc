// SPDX-License-Identifier: MIT

#include <memory>
#include <stdexcept>

#include <boost/test/tools/detail/print_helper.hpp>
#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "nbt/pull_parser/nbt_pull_parser.hh"

using namespace pixel_terrain::nbt;

BOOST_TEST_DONT_PRINT_LOG_VALUE(parser_event);

BOOST_AUTO_TEST_CASE(parser_single_tag_byte) {
    unsigned char data[] = {1, 0, 3, 'f', 'o', 'o', 5}; // NOLINT
    nbt_pull_parser p(data, 7);                         // NOLINT

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_START");
    parser_event ev = p.get_event_type();
    BOOST_TEST(ev == parser_event::DOCUMENT_START);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_type()), std::logic_error);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_name()), std::logic_error);

    BOOST_TEST_CHECKPOINT("parsing TAG_START");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_BYTE);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_START);

    BOOST_TEST_CHECKPOINT("parsing DATA");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_tag_type() == TAG_BYTE);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_byte() == 5);
    BOOST_TEST(p.get_event_type() == parser_event::DATA);

    BOOST_TEST_CHECKPOINT("parsing TAG_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);
    BOOST_TEST(p.get_tag_type() == TAG_BYTE);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_END);

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(parser_single_tag_short) {
    unsigned char data[] = {2, 0, 3, 'f', 'o', 'o', 0, 5}; // NOLINT
    nbt_pull_parser p(data, 8);                            // NOLINT

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_START");
    parser_event ev = p.get_event_type();
    BOOST_TEST(ev == parser_event::DOCUMENT_START);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_type()), std::logic_error);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_name()), std::logic_error);

    BOOST_TEST_CHECKPOINT("parsing TAG_START");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_SHORT);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_START);

    BOOST_TEST_CHECKPOINT("parsing DATA");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_tag_type() == TAG_SHORT);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_short() == 5);
    BOOST_TEST(p.get_event_type() == parser_event::DATA);

    BOOST_TEST_CHECKPOINT("parsing TAG_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);
    BOOST_TEST(p.get_tag_type() == TAG_SHORT);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_END);

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(parser_single_tag_int) {
    unsigned char data[] = {3, 0, 3, 'f', 'o', 'o', 0, 0, 0, 5}; // NOLINT
    nbt_pull_parser p(data, 10);                                 // NOLINT

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_START");
    parser_event ev = p.get_event_type();
    BOOST_TEST(ev == parser_event::DOCUMENT_START);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_type()), std::logic_error);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_name()), std::logic_error);

    BOOST_TEST_CHECKPOINT("parsing TAG_START");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_INT);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_START);

    BOOST_TEST_CHECKPOINT("parsing DATA");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_tag_type() == TAG_INT);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_int() == 5);
    BOOST_TEST(p.get_event_type() == parser_event::DATA);

    BOOST_TEST_CHECKPOINT("parsing TAG_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);
    BOOST_TEST(p.get_tag_type() == TAG_INT);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_END);

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(parser_single_tag_long) {
    // NOLINTNEXTLINEc
    unsigned char data[] = {4, 0, 3, 'f', 'o', 'o', 0, 0, 0, 0, 0, 0, 0, 5};
    nbt_pull_parser p(data, 14); // NOLINT

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_START");
    parser_event ev = p.get_event_type();
    BOOST_TEST(ev == parser_event::DOCUMENT_START);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_type()), std::logic_error);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_name()), std::logic_error);

    BOOST_TEST_CHECKPOINT("parsing TAG_START");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_LONG);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_START);

    BOOST_TEST_CHECKPOINT("parsing DATA");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_tag_type() == TAG_LONG);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_long() == 5);
    BOOST_TEST(p.get_event_type() == parser_event::DATA);

    BOOST_TEST_CHECKPOINT("parsing TAG_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);
    BOOST_TEST(p.get_tag_type() == TAG_LONG);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_END);

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(parser_single_tag_float) {
    // NOLINTNEXTLINE
    unsigned char data[] = {
        // NOLINTNEXTLINE
        5, 0, 3, 'f', 'o', 'o', 0b01000000, 0b10100000, 0b00000000, 0b00000000};
    /*                          5.0 */
    nbt_pull_parser p(data, 10); // NOLINT

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_START");
    parser_event ev = p.get_event_type();
    BOOST_TEST(ev == parser_event::DOCUMENT_START);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_type()), std::logic_error);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_name()), std::logic_error);

    BOOST_TEST_CHECKPOINT("parsing TAG_START");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_FLOAT);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_START);

    BOOST_TEST_CHECKPOINT("parsing DATA");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_tag_type() == TAG_FLOAT);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_float() == 5.0F); // NOLINT
    BOOST_TEST(p.get_event_type() == parser_event::DATA);

    BOOST_TEST_CHECKPOINT("parsing TAG_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);
    BOOST_TEST(p.get_tag_type() == TAG_FLOAT);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_END);

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(parser_single_tag_double) {
    // NOLINTNEXTLINE
    unsigned char data[] = {
        6,          0,          3,          'f',        'o',        // NOLINT
        'o',        0b01000000, 0b00010100, 0b00000000, 0b00000000, // NOLINT
        0b00000000, 0b00000000, 0b00000000, 0b00000000};            // NOLINT
    /*                      5.0 */
    nbt_pull_parser p(data, 14); // NOLINT

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_START");
    parser_event ev = p.get_event_type();
    BOOST_TEST(ev == parser_event::DOCUMENT_START);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_type()), std::logic_error);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_name()), std::logic_error);

    BOOST_TEST_CHECKPOINT("parsing TAG_START");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_DOUBLE);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_START);

    BOOST_TEST_CHECKPOINT("parsing DATA");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_tag_type() == TAG_DOUBLE);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_double() == 5.0); // NOLINT
    BOOST_TEST(p.get_event_type() == parser_event::DATA);

    BOOST_TEST_CHECKPOINT("parsing TAG_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);
    BOOST_TEST(p.get_tag_type() == TAG_DOUBLE);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_END);

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(parser_single_tag_byte_array) {
    // NOLINTNEXTLINE
    unsigned char data[] = {7, 0, 3, 'f', 'o', 'o', 0, 0, 0, 2, 1, 2};
    nbt_pull_parser p(data, 12); // NOLINT

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_START");
    parser_event ev = p.get_event_type();
    BOOST_TEST(ev == parser_event::DOCUMENT_START);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_type()), std::logic_error);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_name()), std::logic_error);

    BOOST_TEST_CHECKPOINT("parsing TAG_START");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_BYTE_ARRAY);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_START);

    BOOST_TEST_CHECKPOINT("parsing DATA");
    unsigned char expected[] = {1, 2}; // NOLINT
    for (unsigned char &b : expected) {
        ev = p.next();
        BOOST_TEST(ev == parser_event::DATA);
        BOOST_TEST(p.get_tag_type() == TAG_BYTE_ARRAY);
        BOOST_TEST(p.get_tag_name() == "foo");
        BOOST_TEST(p.get_byte() == b);
        BOOST_TEST(p.get_event_type() == parser_event::DATA);
    }

    BOOST_TEST_CHECKPOINT("parsing TAG_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);
    BOOST_TEST(p.get_tag_type() == TAG_BYTE_ARRAY);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_END);

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(parser_single_tag_string) {
    // NOLINTNEXTLINE
    unsigned char data[] = {8, 0, 3, 'f', 'o', 'o', 0, 3, 'b', 'a', 'r'};
    nbt_pull_parser p(data, 11); // NOLINT

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_START");
    parser_event ev = p.get_event_type();
    BOOST_TEST(ev == parser_event::DOCUMENT_START);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_type()), std::logic_error);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_name()), std::logic_error);

    BOOST_TEST_CHECKPOINT("parsing TAG_START");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_STRING);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_START);

    BOOST_TEST_CHECKPOINT("parsing DATA");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_tag_type() == TAG_STRING);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_string() == "bar");
    BOOST_TEST(p.get_event_type() == parser_event::DATA);

    BOOST_TEST_CHECKPOINT("parsing TAG_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);
    BOOST_TEST(p.get_tag_type() == TAG_STRING);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_END);

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(parser_single_tag_list_int) {
    // NOLINTNEXTLINE
    unsigned char data[] = {9, 0, 3, 'f', 'o', 'o', 3, 0, 0, 0,
                            2, 0, 0, 0,   1,   0,   0, 0, 2};
    nbt_pull_parser p(data, 19); // NOLINT

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_START");
    parser_event ev = p.get_event_type();
    BOOST_TEST(ev == parser_event::DOCUMENT_START);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_type()), std::logic_error);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_name()), std::logic_error);

    BOOST_TEST_CHECKPOINT("parsing TAG_START");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_LIST);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_START);

    BOOST_TEST_CHECKPOINT("parsing DATA");
    int expected[] = {1, 2}; // NOLINT
    for (int &i : expected) {
        ev = p.next();
        BOOST_TEST(ev == parser_event::TAG_START);
        BOOST_TEST(p.get_tag_type() == TAG_INT);

        ev = p.next();
        BOOST_TEST(ev == parser_event::DATA);
        BOOST_TEST(p.get_tag_type() == TAG_INT);
        BOOST_TEST(p.get_int() == i);
        BOOST_TEST(p.get_event_type() == parser_event::DATA);

        ev = p.next();
        BOOST_TEST(ev == parser_event::TAG_END);
        BOOST_TEST(p.get_tag_type() == TAG_INT);
    }

    BOOST_TEST_CHECKPOINT("parsing TAG_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);
    BOOST_TEST(p.get_tag_type() == TAG_LIST);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_END);

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(parser_simple_tag_compound) {
    // NOLINTNEXTLINE
    unsigned char data[] = {10,  0,   3,   'f', 'o', 'o', 3, 0, 3,
                            'b', 'a', 'r', 0,   0,   0,   3, 0};

    nbt_pull_parser p(data, 17); // NOLINT

    BOOST_TEST_CHECK("parsing DOCUMENT_START");
    parser_event ev = p.get_event_type();
    BOOST_TEST(ev == parser_event::DOCUMENT_START);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_COMPOUND);
    BOOST_TEST(p.get_tag_name() == "foo");

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_INT);
    BOOST_TEST(p.get_tag_name() == "bar");

    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_int() == 3);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(parser_single_tag_int_array) {
    // NOLINTNEXTLINE
    unsigned char data[] = {11, 0, 3, 'f', 'o', 'o', 0, 0, 0,
                            2,  0, 0, 0,   1,   0,   0, 0, 2};
    nbt_pull_parser p(data, 18); // NOLINT

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_START");
    parser_event ev = p.get_event_type();
    BOOST_TEST(ev == parser_event::DOCUMENT_START);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_type()), std::logic_error);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_name()), std::logic_error);

    BOOST_TEST_CHECKPOINT("parsing TAG_START");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_INT_ARRAY);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_START);

    BOOST_TEST_CHECKPOINT("parsing DATA");
    int expected[] = {1, 2}; // NOLINT
    for (int &i : expected) {
        ev = p.next();
        BOOST_TEST(ev == parser_event::DATA);
        BOOST_TEST(p.get_tag_type() == TAG_INT_ARRAY);
        BOOST_TEST(p.get_tag_name() == "foo");
        BOOST_TEST(p.get_int() == i);
        BOOST_TEST(p.get_event_type() == parser_event::DATA);
    }

    BOOST_TEST_CHECKPOINT("parsing TAG_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);
    BOOST_TEST(p.get_tag_type() == TAG_INT_ARRAY);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_END);

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(parser_single_tag_long_array) {
    // NOLINTNEXTLINE
    unsigned char data[] = {12, 0, 3, 'f', 'o', 'o', 0, 0, 0, 2, 0, 0, 0,
                            0,  0, 0, 0,   1,   0,   0, 0, 0, 0, 0, 0, 2};
    nbt_pull_parser p(data, 26); // NOLINT

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_START");
    parser_event ev = p.get_event_type();
    BOOST_TEST(ev == parser_event::DOCUMENT_START);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_type()), std::logic_error);
    BOOST_CHECK_THROW(static_cast<void>(p.get_tag_name()), std::logic_error);

    BOOST_TEST_CHECKPOINT("parsing TAG_START");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_LONG_ARRAY);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_START);

    BOOST_TEST_CHECKPOINT("parsing DATA");
    long expected[] = {1L, 2L}; // NOLINT
    for (long &l : expected) {
        ev = p.next();
        BOOST_TEST(ev == parser_event::DATA);
        BOOST_TEST(p.get_tag_type() == TAG_LONG_ARRAY);
        BOOST_TEST(p.get_tag_name() == "foo");
        BOOST_TEST(p.get_long() == l);
        BOOST_TEST(p.get_event_type() == parser_event::DATA);
    }

    BOOST_TEST_CHECKPOINT("parsing TAG_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);
    BOOST_TEST(p.get_tag_type() == TAG_LONG_ARRAY);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_event_type() == parser_event::TAG_END);

    BOOST_TEST_CHECKPOINT("parsing DOCUMENT_END");
    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(parser_tag_multiple) {
    // NOLINTNEXTLINE
    unsigned char data[] = {1, 0, 3,   'f', 'o', 'o', 3, 2,
                            0, 3, 'b', 'a', 'r', 0,   4};
    nbt_pull_parser p(data, 15); // NOLINT

    parser_event ev = p.get_event_type();
    BOOST_TEST(ev == parser_event::DOCUMENT_START);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_BYTE);
    BOOST_TEST(p.get_tag_name() == "foo");

    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_byte() == 3);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_SHORT);
    BOOST_TEST(p.get_tag_name() == "bar");

    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_short() == 4);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(common_broken_case) {
    // NOLINTNEXTLINE
    unsigned char data[] = {9, 0, 3, 'f', 'o', 'o', 0, 0, 0, 0, 0};
    nbt_pull_parser p(data, 11); // NOLINT

    parser_event ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_LIST);
    BOOST_TEST(p.get_tag_name() == "foo");

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(nested_list) {
    // NOLINTNEXTLINE
    unsigned char data[] = {9, 0, 3, 'f', 'o', 'o', 9, 0, 0,
                            0, 1, 1, 0,   0,   0,   1, 3};
    nbt_pull_parser p(data, 17); // NOLINT

    parser_event ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_LIST);
    BOOST_TEST(p.get_tag_name() == "foo");

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_LIST);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_BYTE);

    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_byte() == 3);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(parser_complex_1) {
    // NOLINTNEXTLINE
    unsigned char data[] = {10, 0, 3,   'f', 'o', 'o', 10,
                            0,  3, 'b', 'a', 'r', 0,   0};
    nbt_pull_parser p(data, 14); // NOLINT

    parser_event ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_COMPOUND);
    BOOST_TEST(p.get_tag_name() == "foo");

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_COMPOUND);
    BOOST_TEST(p.get_tag_name() == "bar");

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(parser_complex_2) {
    // NOLINTNEXTLINE
    unsigned char data[] = {
        9,   0, 3, 'f', 'o', 'o', 10,  0,   0,   0,                  // NOLINT
        2,   1, 0, 3,   'b', 'a', 'r', 1,   1,   0,   3,   'b', 'a', // NOLINT
        'z', 2, 0, 1,   0,   6,   'f', 'o', 'o', 'b', 'a', 'r', 3,   // NOLINT
        0};                                                          // NOLINT
    nbt_pull_parser p(data, 37);                                     // NOLINT

    parser_event ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_LIST);
    BOOST_TEST(p.get_tag_name() == "foo");

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_COMPOUND);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_BYTE);
    BOOST_TEST(p.get_tag_name() == "bar");

    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_byte() == 1);
    ;

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_BYTE);
    BOOST_TEST(p.get_tag_name() == "baz");

    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_byte() == 2);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_COMPOUND);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_BYTE);
    BOOST_TEST(p.get_tag_name() == "foobar");

    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_byte() == 3);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}

BOOST_AUTO_TEST_CASE(parser_complex_3) {
    // NOLINTNEXTLINE
    unsigned char data[] = {10, 0, 3, 'f', 'o', 'o', 9,   0, 3, 'b', 'a', 'r',
                            3,  0, 0, 0,   2,   0,   0,   0, 1, 0,   0,   0,
                            2,  3, 0, 3,   'b', 'a', 'z', 0, 0, 0,   3,   0};
    nbt_pull_parser p(data, 36); // NOLINT

    parser_event ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_name() == "foo");
    BOOST_TEST(p.get_tag_type() == TAG_COMPOUND);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_LIST);
    BOOST_TEST(p.get_tag_name() == "bar");

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_INT);

    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_int() == 1);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_INT);

    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_int() == 2);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_START);
    BOOST_TEST(p.get_tag_type() == TAG_INT);
    BOOST_TEST(p.get_tag_name() == "baz");

    ev = p.next();
    BOOST_TEST(ev == parser_event::DATA);
    BOOST_TEST(p.get_int() == 3);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::TAG_END);

    ev = p.next();
    BOOST_TEST(ev == parser_event::DOCUMENT_END);
}
