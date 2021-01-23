// SPDX-License-Identifier: MIT

#include <iostream>
#include <string>
#include <vector>

#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "nbt/nbt-path.hh"

using namespace pixel_terrain::nbt;

namespace pixel_terrain::nbt {
    auto operator<<(std::ostream &out, enum nbt_path::pathspec::category cat)
        -> std::ostream & {
        std::string repr;
        switch (cat) {
        case nbt_path::pathspec::category::INDEX:
            repr = "category::INDEX";
            break;
        case nbt_path::pathspec::category::KEY:
            repr = "category::KEY";
            break;
        default:
            repr = "category::<invalid category>";
            break;
        }
        return out << repr;
    }

    auto operator<<(std::ostream &out, enum nbt_path::pathspec::container cont)
        -> std::ostream & {
        std::string repr;
        switch (cont) {
        case nbt_path::pathspec::container::NONE:
            repr = "container::NONE";
            break;
        case nbt_path::pathspec::container::ARRAY:
            repr = "container::ARRAY";
            break;
        case nbt_path::pathspec::container::LIST:
            repr = "container::LIST";
            break;
        default:
            repr = "container::<invalid container>";
            break;
        }
        return out << repr;
    }
} // namespace pixel_terrain::nbt

BOOST_AUTO_TEST_CASE(npath_single) {
    auto *p = nbt_path::compile("/foo");
    BOOST_TEST(p != nullptr);
    std::vector<nbt_path::pathspec> path = p->path();
    BOOST_TEST(path.size() == 1);
    BOOST_TEST(path[0].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[0].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[0].key() == "foo");
    delete p;
}

BOOST_AUTO_TEST_CASE(indexed_access_array) {
    auto *p = nbt_path::compile("/foo[123]");
    BOOST_TEST(p != nullptr);
    std::vector<nbt_path::pathspec> path = p->path();
    BOOST_TEST(path.size() == 2);
    BOOST_TEST(path[0].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[0].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[0].key() == "foo");
    BOOST_TEST(path[1].category() == nbt_path::pathspec::category::INDEX);
    BOOST_TEST(path[1].container() == nbt_path::pathspec::container::ARRAY);
    BOOST_TEST(path[1].index() == 123); // NOLINT
}

BOOST_AUTO_TEST_CASE(indexed_access_list) {
    auto *p = nbt_path::compile("/foo<123>");
    BOOST_TEST(p != nullptr);
    std::vector<nbt_path::pathspec> path = p->path();
    BOOST_TEST(path.size() == 2);
    BOOST_TEST(path[0].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[0].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[0].key() == "foo");
    BOOST_TEST(path[1].category() == nbt_path::pathspec::category::INDEX);
    BOOST_TEST(path[1].container() == nbt_path::pathspec::container::LIST);
    BOOST_TEST(path[1].index() == 123); // NOLINT
}

BOOST_AUTO_TEST_CASE(compound_1) {
    auto *p = nbt_path::compile("/foo[1]/bar");
    BOOST_TEST(p != nullptr);
    std::vector<nbt_path::pathspec> path = p->path();
    BOOST_TEST(path.size() == 3);
    BOOST_TEST(path[0].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[0].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[0].key() == "foo");
    BOOST_TEST(path[1].category() == nbt_path::pathspec::category::INDEX);
    BOOST_TEST(path[1].container() == nbt_path::pathspec::container::ARRAY);
    BOOST_TEST(path[1].index() == 1);
    BOOST_TEST(path[2].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[2].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[2].key() == "bar");
}

BOOST_AUTO_TEST_CASE(compound_2) {
    auto *p = nbt_path::compile("/foo/<1>/");
    BOOST_TEST(p != nullptr);
    std::vector<nbt_path::pathspec> path = p->path();
    BOOST_TEST(path.size() == 4);
    BOOST_TEST(path[0].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[0].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[0].key() == "foo");
    BOOST_TEST(path[1].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[1].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[1].key().empty());
    BOOST_TEST(path[2].category() == nbt_path::pathspec::category::INDEX);
    BOOST_TEST(path[2].container() == nbt_path::pathspec::container::LIST);
    BOOST_TEST(path[2].index() == 1);
    BOOST_TEST(path[3].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[3].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[3].key().empty());
}

BOOST_AUTO_TEST_CASE(compound_3) {
    auto *p = nbt_path::compile("/foo//[1]//");
    BOOST_TEST(p != nullptr);
    std::vector<nbt_path::pathspec> path = p->path();
    BOOST_TEST(path.size() == 6);
    BOOST_TEST(path[0].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[0].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[0].key() == "foo");
    BOOST_TEST(path[1].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[1].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[1].key().empty());
    BOOST_TEST(path[2].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[2].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[2].key().empty());
    BOOST_TEST(path[3].category() == nbt_path::pathspec::category::INDEX);
    BOOST_TEST(path[3].container() == nbt_path::pathspec::container::ARRAY);
    BOOST_TEST(path[3].index() == 1);
    BOOST_TEST(path[4].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[4].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[4].key().empty());
    BOOST_TEST(path[5].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[5].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[5].key().empty());
}

BOOST_AUTO_TEST_CASE(pop) {
    auto *p = nbt_path::compile("/foo/bar/baz");
    BOOST_TEST(p != nullptr);
    BOOST_TEST(p->remain());
    BOOST_TEST(p->get_one().key() == "foo");
    BOOST_TEST(p->remain());
    BOOST_TEST(p->get_one().key() == "bar");
    BOOST_TEST(p->remain());
    BOOST_TEST(p->get_one().key() == "baz");
    BOOST_TEST(!p->remain());
    delete p;
}
