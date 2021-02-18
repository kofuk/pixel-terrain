// SPDX-License-Identifier: MIT

#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "nbt/nbt-path.hh"
#include "nbt/nbt.hh"
#include "nbt/tag.hh"
#include "nbt_test_testdata.hh"

using namespace pixel_terrain::nbt;

struct F {
    pixel_terrain::nbt::nbt *file;

    F() {
        auto data = *get_embedded_data("nbt-file.nbt");
        file = nbt::from_iterator(data.begin(), data.end());
    }

    ~F() { delete file; }
};

BOOST_FIXTURE_TEST_SUITE(query, F)

    BOOST_AUTO_TEST_CASE(single_value_1) {
        auto path = nbt_path::compile("//foo/foo1");
        auto *tag = file->query<tag_byte_payload>(path);
        BOOST_TEST(tag != nullptr);
        BOOST_TEST(**tag == 2);
        delete tag;
    }

    BOOST_AUTO_TEST_CASE(single_value_2) {
        auto path = nbt_path::compile("//foo/foo2");
        auto *tag = file->query<tag_byte_payload>(path);
        BOOST_TEST(tag != nullptr);
        BOOST_TEST(**tag == 1);
        delete tag;
    }

    BOOST_AUTO_TEST_CASE(single_value_3) {
        auto path = nbt_path::compile("//foo/foo3");
        auto *tag = file->query<tag_string_payload>(path);
        BOOST_TEST(tag != nullptr);
        BOOST_TEST(**tag == "Hello!");
        delete tag;
    }

    BOOST_AUTO_TEST_CASE(single_value_4) {
        auto path = nbt_path::compile("//baz/foo");
        auto *tag = file->query<tag_int_payload>(path);
        BOOST_TEST(tag != nullptr);
        BOOST_TEST(**tag == 3);
        delete tag;
    }

    BOOST_AUTO_TEST_CASE(single_value_wrong_type) {
        auto path = nbt_path::compile("//foo/foo1");
        auto *tag = file->query<tag_int_payload>(path);
        BOOST_TEST(tag == nullptr);
    }

    BOOST_AUTO_TEST_CASE(single_value_nonexisting) {
        auto path = nbt_path::compile("//foo/ZZZ");
        auto *tag = file->query<tag_int_payload>(path);
        BOOST_TEST(tag == nullptr);
    }

    BOOST_AUTO_TEST_CASE(array_type_1) {
        auto path = nbt_path::compile("//bar");
        auto *tag = file->query<tag_int_array_payload>(path);
        BOOST_TEST(tag != nullptr);
        BOOST_TEST((*tag)->size() == 3);
        BOOST_TEST((**tag)[0] == 3);
        BOOST_TEST((**tag)[1] == 1);
        BOOST_TEST((**tag)[2] == 4);
        delete tag;
    }

    BOOST_AUTO_TEST_CASE(array_type_2) {
        auto path = nbt_path::compile("//baz/bar/abc");
        auto *tag = file->query<tag_byte_array_payload>(path);
        BOOST_TEST(tag != nullptr);
        BOOST_TEST((*tag)->size() == 5);
        BOOST_TEST((**tag)[0] == 1);
        BOOST_TEST((**tag)[1] == 4);
        BOOST_TEST((**tag)[2] == 1);
        BOOST_TEST((**tag)[3] == 4);
        BOOST_TEST((**tag)[4] == 2);
        delete tag;
    }

    BOOST_AUTO_TEST_CASE(array_type_3) {
        auto path = nbt_path::compile("//baz/bar/def");
        auto *tag = file->query<tag_byte_array_payload>(path);
        BOOST_TEST(tag != nullptr);
        BOOST_TEST((*tag)->size() == 4);
        BOOST_TEST((**tag)[0] == 1);
        BOOST_TEST((**tag)[1] == 7);
        BOOST_TEST((**tag)[2] == 3);
        BOOST_TEST((**tag)[3] == 2);
        delete tag;
    }

    BOOST_AUTO_TEST_CASE(array_element) {
        auto path = nbt_path::compile("//baz/bar/def[2]");
        auto *tag = file->query<tag_byte_payload>(path);
        BOOST_TEST(tag != nullptr);
        BOOST_TEST(**tag == 3);
        delete tag;
    }

    BOOST_AUTO_TEST_CASE(list_element) {
        auto path = nbt_path::compile("//baz/bar/ghi<1>");
        auto *tag = file->query<tag_int_payload>(path);
        BOOST_TEST(tag != nullptr);
        BOOST_TEST(**tag == 2);
        delete tag;
    }

    BOOST_AUTO_TEST_CASE(array_element_wrong_container_type) {
        auto path = nbt_path::compile("//baz/bar/def<2>");
        auto *tag = file->query<tag_byte_payload>(path);
        BOOST_TEST(tag == nullptr);
    }

    BOOST_AUTO_TEST_CASE(list_element_wrong_container_type) {
        auto path = nbt_path::compile("//baz/bar/ghi[1]");
        auto *tag = file->query<tag_int_payload>(path);
        BOOST_TEST(tag == nullptr);
    }

    BOOST_AUTO_TEST_CASE(subtree) {
        auto path = nbt_path::compile("//baz");
        auto *tag = file->query<tag_compound_payload>(path);
        BOOST_TEST(tag != nullptr);
        path = nbt_path::compile("/foo");
        auto *inner = tag->query<tag_int_payload>(path);
        BOOST_TEST(inner != nullptr);
        BOOST_TEST(**inner == 3);
        delete inner;
        delete tag;
    }

BOOST_AUTO_TEST_SUITE_END()
