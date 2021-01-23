// SPDX-License-Identifier: MIT

#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "nbt/nbt-path.hh"
#include "nbt/nbt.hh"
#include "nbt/tag.hh"
#include "nbt_test_testdata.hh"

struct F {
    pixel_terrain::nbt::nbt *file;

    F() {
        auto data = *get_embedded_data("nbt-file.nbt");
        file = pixel_terrain::nbt::nbt::from_iterator(data.begin(), data.end());
    }

    ~F() { delete file; }
};

BOOST_FIXTURE_TEST_SUITE(query, F)

    BOOST_AUTO_TEST_CASE(test_query) {
        auto *path = pixel_terrain::nbt::nbt_path::compile("//baz/foo");
        auto tag = file->query<pixel_terrain::nbt::tag_int_payload>(*path);
        BOOST_TEST(tag != nullptr);
        BOOST_TEST(**tag == 2);
    }

BOOST_AUTO_TEST_SUITE_END()
