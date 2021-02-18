// SPDX-License-Identifier: MIT

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <sstream>
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
        return out << tag_type_repr(ty);
    }
} // namespace pixel_terrain::nbt

BOOST_AUTO_TEST_SUITE(parser)

    BOOST_AUTO_TEST_CASE(single_tag_byte) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-byte.nbt");

        {
            auto [t, itr] =
                nbt::tag_byte::parse_buffer(data.begin(), data.end());
            (void)itr;
            BOOST_TEST(t != nullptr);
            BOOST_TEST(t->name() == "foo");
            BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_BYTE);
            BOOST_TEST(**t->typed_data<nbt::tag_byte_payload>() == 5);
            delete t;
        }

        {
            auto [t, itr] =
                nbt::tag_end::parse_buffer(data.begin(), data.end());
            (void)itr;
            BOOST_TEST(t == nullptr);
            delete t;
        }

        {
            auto [t, itr] =
                nbt::tag_short::parse_buffer(data.begin(), data.end());
            (void)itr;
            BOOST_TEST(t == nullptr);
            delete t;
        }

        {
            auto [t, itr] =
                nbt::tag_int::parse_buffer(data.begin(), data.end());
            (void)itr;
            BOOST_TEST(t == nullptr);
            delete t;
        }

        {
            auto [t, itr] =
                nbt::tag_long::parse_buffer(data.begin(), data.end());
            (void)itr;
            BOOST_TEST(t == nullptr);
            delete t;
        }

        {
            auto [t, itr] =
                nbt::tag_float::parse_buffer(data.begin(), data.end());
            (void)itr;
            BOOST_TEST(t == nullptr);
            delete t;
        }

        {
            auto [t, itr] =
                nbt::tag_double::parse_buffer(data.begin(), data.end());
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
            auto [t, itr] =
                nbt::tag_string::parse_buffer(data.begin(), data.end());
            (void)itr;
            BOOST_TEST(t == nullptr);
            delete t;
        }

        {
            auto [t, itr] =
                nbt::tag_list::parse_buffer(data.begin(), data.end());
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

    BOOST_AUTO_TEST_CASE(single_tag_short) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-short.nbt");

        {
            auto [t, itr] =
                nbt::tag_short::parse_buffer(data.begin(), data.end());
            (void)itr;
            BOOST_TEST(t != nullptr);
            BOOST_TEST(t->name() == "foo");
            BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_SHORT);
            BOOST_TEST(**t->typed_data<nbt::tag_short_payload>() == 5);
            delete t;
        }

        {
            auto [t, itr] =
                nbt::tag_byte::parse_buffer(data.begin(), data.end());
            (void)itr;
            BOOST_TEST(t == nullptr);
            delete t;
        }
    }

    BOOST_AUTO_TEST_CASE(single_tag_int) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-int.nbt");

        auto [t, itr] = nbt::tag_int::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t != nullptr);
        BOOST_TEST(t->name() == "foo");
        BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_INT);
        BOOST_TEST(**t->typed_data<nbt::tag_int_payload>() == 5);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(single_tag_long) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-long.nbt");

        auto [t, itr] = nbt::tag_long::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t != nullptr);
        BOOST_TEST(t->name() == "foo");
        BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_LONG);
        BOOST_TEST(**t->typed_data<nbt::tag_long_payload>() == 5);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(single_tag_float) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-float.nbt");

        auto [t, itr] = nbt::tag_float::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t != nullptr);
        BOOST_TEST(t->name() == "foo");
        BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_FLOAT);
        constexpr float expected_value = 5.0F;
        BOOST_TEST(**t->typed_data<nbt::tag_float_payload>() == expected_value);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(single_tag_double) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-double.nbt");

        auto [t, itr] = nbt::tag_double::parse_buffer(data.begin(), data.end());
        (void)itr;
        BOOST_TEST(t != nullptr);
        BOOST_TEST(t->name() == "foo");
        BOOST_TEST(t->data()->type() == nbt::tag_type::TAG_DOUBLE);
        constexpr double expected_value = 5.0;
        BOOST_TEST(**t->typed_data<nbt::tag_double_payload>() ==
                   expected_value);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(single_tag_byte_array) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-byte-array.nbt");

        auto [t, itr] =
            nbt::tag_byte_array::parse_buffer(data.begin(), data.end());
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

    BOOST_AUTO_TEST_CASE(single_tag_string) {
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

    BOOST_AUTO_TEST_CASE(single_tag_list) {
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

    BOOST_AUTO_TEST_CASE(simple_tag_compound) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("simple-tag-compound.nbt");

        auto [t, itr] =
            nbt::tag_compound::parse_buffer(data.begin(), data.end());
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

    BOOST_AUTO_TEST_CASE(single_tag_int_array) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-int-array.nbt");

        auto [t, itr] =
            nbt::tag_int_array::parse_buffer(data.begin(), data.end());
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

    BOOST_AUTO_TEST_CASE(single_tag_long_array) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-long-array.nbt");

        auto [t, itr] =
            nbt::tag_long_array::parse_buffer(data.begin(), data.end());
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

    BOOST_AUTO_TEST_CASE(tag_multiple) {
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

    BOOST_AUTO_TEST_CASE(broken) {
        std::vector<std::uint8_t> data = *get_embedded_data("broken.nbt");

        auto [t, itr] = nbt::tag_byte::parse_buffer(data.begin(), data.end());
        BOOST_TEST(t == nullptr);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(nested_list) {
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

    BOOST_AUTO_TEST_CASE(complex_1) {
        std::vector<std::uint8_t> data = *get_embedded_data("complex-1.nbt");

        auto [t, itr] =
            nbt::tag_compound::parse_buffer(data.begin(), data.end());
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

    BOOST_AUTO_TEST_CASE(complex_2) {
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

    BOOST_AUTO_TEST_CASE(complex_3) {
        std::vector<std::uint8_t> data = *get_embedded_data("complex-3.nbt");

        auto [t, itr] =
            nbt::tag_compound::parse_buffer(data.begin(), data.end());
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

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(printer)
    BOOST_AUTO_TEST_CASE(single_tag_byte) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-byte.nbt");

        auto [t, itr] = nbt::tag_byte::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_Byte name="foo">
 5
</TAG_Byte>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(single_tag_short) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-short.nbt");

        auto [t, itr] = nbt::tag_short::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_Short name="foo">
 5
</TAG_Short>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(single_tag_int) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-int.nbt");

        auto [t, itr] = nbt::tag_int::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_Int name="foo">
 5
</TAG_Int>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(single_tag_long) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-long.nbt");

        auto [t, itr] = nbt::tag_long::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_Long name="foo">
 5
</TAG_Long>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(single_tag_float) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-float.nbt");

        auto [t, itr] = nbt::tag_float::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_Float name="foo">
 5
</TAG_Float>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(single_tag_double_test) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-double.nbt");

        auto [t, itr] = nbt::tag_double::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_Double name="foo">
 5
</TAG_Double>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(single_tag_byte_array_test) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-byte-array.nbt");

        auto [t, itr] =
            nbt::tag_byte_array::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_Byte_Array name="foo">
 <element>1</element>
 <element>2</element>
</TAG_Byte_Array>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(single_tag_string_test) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-string.nbt");

        auto [t, itr] = nbt::tag_string::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_String name="foo">
 bar
</TAG_String>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(single_tag_list_int) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-list-int.nbt");

        auto [t, itr] = nbt::tag_list::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_List name="foo">
 <payloadtype>TAG_Int</payloadtype>
 <element>
  1
 </element>
 <element>
  2
 </element>
</TAG_List>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(simple_tag_compound_test) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("simple-tag-compound.nbt");

        auto [t, itr] =
            nbt::tag_compound::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_Compound name="foo">
 <TAG_Int name="bar">
  3
 </TAG_Int>
</TAG_Compound>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(single_tag_int_array_test) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-int-array.nbt");

        auto [t, itr] =
            nbt::tag_int_array::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_Int_Array name="foo">
 <element>1</element>
 <element>2</element>
</TAG_Int_Array>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(single_tag_long_array_test) {
        std::vector<std::uint8_t> data =
            *get_embedded_data("single-tag-long-array.nbt");

        auto [t, itr] =
            nbt::tag_long_array::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_Long_Array name="foo">
 <element>1</element>
 <element>2</element>
</TAG_Long_Array>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(nested_list) {
        std::vector<std::uint8_t> data = *get_embedded_data("nested-list.nbt");

        auto [t, itr] = nbt::tag_list::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_List name="foo">
 <payloadtype>TAG_List</payloadtype>
 <element>
  <payloadtype>TAG_Byte</payloadtype>
  <element>
   3
  </element>
 </element>
</TAG_List>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(complex_1) {
        std::vector<std::uint8_t> data = *get_embedded_data("complex-1.nbt");

        auto [t, itr] =
            nbt::tag_compound::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_Compound name="foo">
 <TAG_Compound name="bar">
 </TAG_Compound>
</TAG_Compound>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(complex_2) {
        std::vector<std::uint8_t> data = *get_embedded_data("complex-2.nbt");

        auto [t, itr] = nbt::tag_list::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_List name="foo">
 <payloadtype>TAG_Compound</payloadtype>
 <element>
  <TAG_Byte name="bar">
   1
  </TAG_Byte>
  <TAG_Byte name="baz">
   2
  </TAG_Byte>
 </element>
 <element>
  <TAG_Byte name="foobar">
   3
  </TAG_Byte>
 </element>
</TAG_List>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

    BOOST_AUTO_TEST_CASE(complex_3) {
        std::vector<std::uint8_t> data = *get_embedded_data("complex-3.nbt");

        auto [t, itr] =
            nbt::tag_compound::parse_buffer(data.begin(), data.end());
        (void)itr;
        std::stringstream strm;
        t->repr(strm, " ", 0);
        std::string expected = R"(<TAG_Compound name="foo">
 <TAG_List name="bar">
  <payloadtype>TAG_Int</payloadtype>
  <element>
   1
  </element>
  <element>
   2
  </element>
 </TAG_List>
 <TAG_Int name="baz">
  3
 </TAG_Int>
</TAG_Compound>
)";
        BOOST_TEST(strm.str() == expected);
        delete t;
    }

BOOST_AUTO_TEST_SUITE_END()
