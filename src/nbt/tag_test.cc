// SPDX-License-Identifier: MIT

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "nbt/tag.hh"
#include "nbt_test_testdata.hh"

using namespace pixel_terrain;

namespace pixel_terrain::nbt {
    auto operator<<(std::ostream &out, tag_type ty) -> std::ostream & {
        std::string string_repr;
        switch (ty) {
        case tag_type::TAG_END:
            string_repr = "TAG_END";
            break;
        case tag_type::TAG_BYTE:
            string_repr = "TAG_BYTE";
            break;
        case tag_type::TAG_SHORT:
            string_repr = "TAG_SHORT";
            break;
        case tag_type::TAG_INT:
            string_repr = "TAG_INT";
            break;
        case tag_type::TAG_LONG:
            string_repr = "TAG_LONG";
            break;
        case tag_type::TAG_FLOAT:
            string_repr = "TAG_FLOAT";
            break;
        case tag_type::TAG_DOUBLE:
            string_repr = "TAG_DOUBLE";
            break;
        case tag_type::TAG_BYTE_ARRAY:
            string_repr = "TAG_BYTE_ARRAY";
            break;
        case tag_type::TAG_STRING:
            string_repr = "TAG_STRING";
            break;
        case tag_type::TAG_LIST:
            string_repr = "TAG_LIST";
            break;
        case tag_type::TAG_COMPOUND:
            string_repr = "TAG_COMPOUND";
            break;
        case tag_type::TAG_INT_ARRAY:
            string_repr = "TAG_INT_ARRAY";
            break;
        case tag_type::TAG_LONG_ARRAY:
            string_repr = "TAG_LONG_ARRAY";
            break;
        default:
            string_repr = "<invalid tag>";
        }

        return out << string_repr;
    }
} // namespace pixel_terrain::nbt

BOOST_AUTO_TEST_CASE(single_tag_byte_test) {
    std::vector<std::uint8_t> data = *get_embedded_data("single-tag-byte.nbt");

    {
        auto [t, itr] = nbt::tag_byte::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t != nullptr);
        BOOST_TEST(t->name() == "foo");
        BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_BYTE);
        BOOST_TEST(**t->typed_data<nbt::tag_byte_payload>() == 5);
        delete t;
    }

    {
        auto [t, itr] = nbt::tag_end::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t == nullptr);
        delete t;
    }

    {
        auto [t, itr] = nbt::tag_short::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t == nullptr);
        delete t;
    }

    {
        auto [t, itr] = nbt::tag_int::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t == nullptr);
        delete t;
    }

    {
        auto [t, itr] = nbt::tag_long::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t == nullptr);
        delete t;
    }

    {
        auto [t, itr] = nbt::tag_float::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t == nullptr);
        delete t;
    }

    {
        auto [t, itr] = nbt::tag_double::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t == nullptr);
        delete t;
    }

    {
        auto [t, itr] =
            nbt::tag_byte_array::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t == nullptr);
        delete t;
    }

    {
        auto [t, itr] = nbt::tag_string::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t == nullptr);
        delete t;
    }

    {
        auto [t, itr] = nbt::tag_list::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t == nullptr);
        delete t;
    }

    {
        auto [t, itr] =
            nbt::tag_compound::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t == nullptr);
        delete t;
    }

    {
        auto [t, itr] =
            nbt::tag_int_array::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t == nullptr);
        delete t;
    }

    {
        auto [t, itr] =
            nbt::tag_long_array::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t == nullptr);
        delete t;
    }
}

BOOST_AUTO_TEST_CASE(single_tag_short_test) {
    std::vector<std::uint8_t> data = *get_embedded_data("single-tag-short.nbt");

    {
        auto [t, itr] = nbt::tag_short::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t != nullptr);
        BOOST_TEST(t->name() == "foo");
        BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_SHORT);
        BOOST_TEST(**t->typed_data<nbt::tag_short_payload>() == 5);
        delete t;
    }

    {
        auto [t, itr] = nbt::tag_byte::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t == nullptr);
        delete t;
    }
}

BOOST_AUTO_TEST_CASE(single_tag_int_test) {
    std::vector<std::uint8_t> data = *get_embedded_data("single-tag-int.nbt");

    auto [t, itr] = nbt::tag_int::parse_buffer(data.begin(), data.end());
    (void)itr;
    BOOST_TEST(t != nullptr);
    BOOST_TEST(t->name() == "foo");
    BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_INT);
    BOOST_TEST(**t->typed_data<nbt::tag_int_payload>() == 5);
    delete t;
}

BOOST_AUTO_TEST_CASE(single_tag_long_test) {
    std::vector<std::uint8_t> data = *get_embedded_data("single-tag-long.nbt");

    auto [t, itr] = nbt::tag_long::parse_buffer(data.begin(), data.end());
    (void)itr;
    BOOST_TEST(t != nullptr);
    BOOST_TEST(t->name() == "foo");
    BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_LONG);
    BOOST_TEST(**t->typed_data<nbt::tag_long_payload>() == 5);
    delete t;
}

BOOST_AUTO_TEST_CASE(single_tag_float_test) {
    std::vector<std::uint8_t> data = *get_embedded_data("single-tag-float.nbt");

    auto [t, itr] = nbt::tag_float::parse_buffer(data.begin(), data.end());
    (void)itr;
    BOOST_TEST(t != nullptr);
    BOOST_TEST(t->name() == "foo");
    BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_FLOAT);
    constexpr float expected_value = 5.0F;
    BOOST_TEST(**t->typed_data<nbt::tag_float_payload>() == expected_value);
    delete t;
}

BOOST_AUTO_TEST_CASE(single_tag_double_test) {
    std::vector<std::uint8_t> data =
        *get_embedded_data("single-tag-double.nbt");

    auto [t, itr] = nbt::tag_double::parse_buffer(data.begin(), data.end());
    (void)itr;
    BOOST_TEST(t != nullptr);
    BOOST_TEST(t->name() == "foo");
    BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_DOUBLE);
    constexpr double expected_value = 5.0;
    BOOST_TEST(**t->typed_data<nbt::tag_double_payload>() == expected_value);
    delete t;
}

BOOST_AUTO_TEST_CASE(single_tag_byte_array_test) {
    std::vector<std::uint8_t> data =
        *get_embedded_data("single-tag-byte-array.nbt");

    auto [t, itr] = nbt::tag_byte_array::parse_buffer(data.begin(), data.end());
    (void)itr;
    BOOST_TEST(t != nullptr);
    BOOST_TEST(t->name() == "foo");
    BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_BYTE_ARRAY);
    auto *payload = t->typed_data<nbt::tag_byte_array_payload>();
    BOOST_TEST((*payload)->size() == 2);
    BOOST_TEST((*payload)[0] == 1);
    BOOST_TEST((*payload)[1] == 2);
    delete t;
}

BOOST_AUTO_TEST_CASE(single_tag_string_test) {
    std::vector<std::uint8_t> data =
        *get_embedded_data("single-tag-string.nbt");

    auto [t, itr] = nbt::tag_string::parse_buffer(data.begin(), data.end());
    (void)itr;
    BOOST_TEST(t != nullptr);
    BOOST_TEST(t->name() == "foo");
    BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_STRING);
    BOOST_TEST(**t->typed_data<nbt::tag_string_payload>() == "bar");
    delete t;
}

BOOST_AUTO_TEST_CASE(single_tag_list_int) {
    std::vector<std::uint8_t> data =
        *get_embedded_data("single-tag-list-int.nbt");

    auto [t, itr] = nbt::tag_list::parse_buffer(data.begin(), data.end());
    (void)itr;
    BOOST_TEST(t != nullptr);
    BOOST_TEST(t->name() == "foo");
    BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_LIST);
    auto *payload = t->typed_data<nbt::tag_list_payload>();
    BOOST_TEST(payload->payload_type() == nbt::tag_type::TAG_INT);
    BOOST_TEST(**payload->get<nbt::tag_int_payload>(0) == 1);
    BOOST_TEST(**payload->get<nbt::tag_int_payload>(1) == 2);
    delete t;
}

BOOST_AUTO_TEST_CASE(simple_tag_compound_test) {
    std::vector<std::uint8_t> data =
        *get_embedded_data("simple-tag-compound.nbt");

    auto [t, itr] = nbt::tag_compound::parse_buffer(data.begin(), data.end());
    (void)itr;
    BOOST_TEST(t != nullptr);
    BOOST_TEST(t->name() == "foo");
    BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_COMPOUND);
    auto *payload = t->typed_data<nbt::tag_compound_payload>();
    BOOST_TEST((*payload)->size() == 1);
    BOOST_TEST((*payload)[0]->type() == nbt::tag_type::TAG_INT);
    auto *inner_tag = static_cast<nbt::tag_int *>((*payload)[0]);
    BOOST_TEST(inner_tag->name() == "bar");
    BOOST_TEST(**inner_tag->typed_data<nbt::tag_int_payload>() == 3);

    delete t;
}

BOOST_AUTO_TEST_CASE(single_tag_int_array_test) {
    std::vector<std::uint8_t> data =
        *get_embedded_data("single-tag-int-array.nbt");

    auto [t, itr] = nbt::tag_int_array::parse_buffer(data.begin(), data.end());
    (void)itr;
    BOOST_TEST(t != nullptr);
    BOOST_TEST(t->name() == "foo");
    BOOST_TEST(t->type() == nbt::tag_type::TAG_INT_ARRAY);
    auto *payload = t->typed_data<nbt::tag_int_array_payload>();
    BOOST_TEST((*payload)->size() == 2);
    BOOST_TEST((*payload)[0] == 1);
    BOOST_TEST((*payload)[1] == 2);

    delete t;
}

BOOST_AUTO_TEST_CASE(single_tag_long_array_test) {
    std::vector<std::uint8_t> data =
        *get_embedded_data("single-tag-long-array.nbt");

    auto [t, itr] = nbt::tag_long_array::parse_buffer(data.begin(), data.end());
    (void)itr;
    BOOST_TEST(t != nullptr);
    BOOST_TEST(t->name() == "foo");
    BOOST_TEST(t->type() == nbt::tag_type::TAG_LONG_ARRAY);
    auto *payload = t->typed_data<nbt::tag_long_array_payload>();
    BOOST_TEST((*payload)->size() == 2);
    BOOST_TEST((*payload)[0] == 1L);
    BOOST_TEST((*payload)[1] == 2L);

    delete t;
}

BOOST_AUTO_TEST_CASE(tag_multiple_test) {
    std::vector<std::uint8_t> data = *get_embedded_data("tag-multiple.nbt");

    auto [t1, itr] = nbt::tag_byte::parse_buffer(data.begin(), data.end());
    BOOST_TEST(t1 != nullptr);
    BOOST_TEST(t1->name() == "foo");
    BOOST_TEST(**t1->typed_data<nbt::tag_byte_payload>() == 3);
    delete t1;

    auto [t2, itr_2] = nbt::tag_short::parse_buffer(itr, data.end());
    (void)itr_2;
    BOOST_TEST(t2 != nullptr);
    BOOST_TEST(t2->name() == "bar");
    BOOST_TEST(**t1->typed_data<nbt::tag_short_payload>() == 4);
    delete t1;
}

BOOST_AUTO_TEST_CASE(broken_test) {
    std::vector<std::uint8_t> data = *get_embedded_data("broken.nbt");

    auto [t, itr] = nbt::tag_byte::parse_buffer(data.begin(), data.end());
    BOOST_TEST(t == nullptr);
    delete t;
}

BOOST_AUTO_TEST_CASE(nested_list_test) {
    std::vector<std::uint8_t> data = *get_embedded_data("nested-list.nbt");

    auto [t, itr] = nbt::tag_list::parse_buffer(data.begin(), data.end());
    (void)itr;
    BOOST_TEST(t != nullptr);
    BOOST_TEST(t->name() == "foo");
    BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_LIST);
    auto *payload1 = t->typed_data<nbt::tag_list_payload>();

    BOOST_TEST((*payload1)->size() == 1);
    BOOST_TEST((*payload1).payload_type() == nbt::tag_type::TAG_LIST);
    auto *payload2 = payload1->get<nbt::tag_list_payload>(0);
    BOOST_TEST((*payload2)->size() == 1);
    BOOST_TEST(payload2->payload_type() == nbt::tag_type::TAG_BYTE);
    BOOST_TEST(**payload2->get<nbt::tag_byte_payload>(0) == 3);

    delete t;
}

BOOST_AUTO_TEST_CASE(complex_1_test) {
    std::vector<std::uint8_t> data = *get_embedded_data("complex-1.nbt");

    auto [t, itr] = nbt::tag_compound::parse_buffer(data.begin(), data.end());
    (void)itr;
    BOOST_TEST(t != nullptr);
    BOOST_TEST(t->name() == "foo");
    auto *payload = t->typed_data<nbt::tag_compound_payload>();
    BOOST_TEST((*payload)->size() == 1);

    auto *inner_tag = (*payload)["bar"];
    BOOST_TEST(inner_tag != nullptr);
    BOOST_TEST(inner_tag->type() == nbt::tag_type::TAG_COMPOUND);
    auto &&inner_payload = static_cast<nbt::tag_compound *>(inner_tag)
                               ->typed_data<nbt::tag_compound_payload>();
    BOOST_TEST((*inner_payload)->size() == 0);

    delete t;
}

BOOST_AUTO_TEST_CASE(complex_2_test) {
    std::vector<std::uint8_t> data = *get_embedded_data("complex-2.nbt");

    auto [t, itr] = nbt::tag_list::parse_buffer(data.begin(), data.end());
    (void)itr;
    BOOST_TEST(t != nullptr);
    BOOST_TEST(t->name() == "foo");

    auto payload_root = t->typed_data<nbt::tag_list_payload>();
    BOOST_TEST(payload_root->payload_type() == nbt::tag_type::TAG_COMPOUND);
    BOOST_TEST((*payload_root)->size() == 2);

    auto *payload_1 = payload_root->get<nbt::tag_compound_payload>(0);
    BOOST_TEST((*payload_1)->size() == 2);

    auto *tag_1_1 = (*payload_1)["bar"];
    BOOST_TEST(tag_1_1 != nullptr);
    BOOST_TEST(tag_1_1->type() == nbt::tag_type::TAG_BYTE);
    BOOST_TEST(**(*static_cast<nbt::tag_byte *>(tag_1_1))
                     .typed_data<nbt::tag_byte_payload>() == 1);

    auto *tag_1_2 = (*payload_1)["baz"];
    BOOST_TEST(tag_1_2 != nullptr);
    BOOST_TEST(tag_1_2->type() == nbt::tag_type::TAG_BYTE);
    BOOST_TEST(**(*static_cast<nbt::tag_byte *>(tag_1_2))
                     .typed_data<nbt::tag_byte_payload>() == 2);

    auto *payload_2 = payload_root->get<nbt::tag_compound_payload>(1);
    BOOST_TEST((*payload_2)->size() == 1);
    auto *tag_2_1 = (*payload_2)["foobar"];
    BOOST_TEST(tag_2_1 != nullptr);
    BOOST_TEST(tag_2_1->type() == nbt::tag_type::TAG_BYTE);
    BOOST_TEST(**(*static_cast<nbt::tag_byte *>(tag_2_1))
                     .typed_data<nbt::tag_byte_payload>() == 3);

    delete t;
}

BOOST_AUTO_TEST_CASE(complex_3_test) {
    std::vector<std::uint8_t> data = *get_embedded_data("complex-3.nbt");

    auto [t, itr] = nbt::tag_compound::parse_buffer(data.begin(), data.end());
    (void)itr;
    BOOST_TEST(t != nullptr);
    BOOST_TEST(t->name() == "foo");

    auto *payload_root = t->typed_data<nbt::tag_compound_payload>();
    BOOST_TEST((*payload_root)->size() == 2);

    nbt::tag *inner_1 = (*payload_root)["bar"];
    BOOST_TEST(inner_1 != nullptr);
    BOOST_TEST(inner_1->type() == nbt::tag_type::TAG_LIST);

    auto &&payload_1 = static_cast<nbt::tag_list *>(inner_1)
                           ->typed_data<nbt::tag_list_payload>();
    BOOST_TEST((*payload_1)->size() == 2);
    BOOST_TEST(**payload_1->get<nbt::tag_int_payload>(0) == 1);
    BOOST_TEST(**payload_1->get<nbt::tag_int_payload>(1) == 2);

    nbt::tag *inner_2 = (*payload_root)["baz"];
    BOOST_TEST(inner_2 != nullptr);
    BOOST_TEST(inner_2->type() == nbt::tag_type::TAG_INT);
    BOOST_TEST(**static_cast<nbt::tag_int *>(inner_2)
                     ->typed_data<nbt::tag_int_payload>() == 3);

    delete t;
}
