// SPDX-License-Identifier: MIT

#include <iostream>
#include <string>
#include <vector>

#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "nbt/nbt-path.hh"
#include "utils/array.hh"

using namespace pixel_terrain::nbt;

namespace pixel_terrain::nbt {
    namespace {
        auto category_repr =
            make_array<std::string>("category::INDEX", "category::KEY");
        auto container_repr = make_array<std::string>(
            "container::NONE", "container::ARRAY", "container::LIST");
    } // namespace

    auto operator<<(std::ostream &out, enum nbt_path::pathspec::category cat)
        -> std::ostream & {
        if (static_cast<int>(cat) < 2) {
            return out << category_repr[static_cast<int>(cat)];
        } else {
            return out << "category::<invalid category>";
        }
    }

    auto operator<<(std::ostream &out, enum nbt_path::pathspec::container cont)
        -> std::ostream & {
        if (static_cast<int>(cont) < 3) {
            return out << container_repr[static_cast<int>(cont)];
        } else {
            return out << "container::<invalid container>";
        }
    }
} // namespace pixel_terrain::nbt

BOOST_AUTO_TEST_CASE(npath_single) {
    auto p = nbt_path::compile("/foo");
    BOOST_TEST(p);
    std::vector<nbt_path::pathspec> path = p.path();
    BOOST_TEST(path.size() == 1);
    BOOST_TEST(path[0].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[0].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[0].key() == "foo");
}

BOOST_AUTO_TEST_CASE(indexed_access_array) {
    auto p = nbt_path::compile("/foo[123]");
    BOOST_TEST(p);
    std::vector<nbt_path::pathspec> path = p.path();
    BOOST_TEST(path.size() == 2);
    BOOST_TEST(path[0].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[0].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[0].key() == "foo");
    BOOST_TEST(path[1].category() == nbt_path::pathspec::category::INDEX);
    BOOST_TEST(path[1].container() == nbt_path::pathspec::container::ARRAY);
    BOOST_TEST(path[1].index() == 123); // NOLINT
}

BOOST_AUTO_TEST_CASE(indexed_access_list) {
    auto p = nbt_path::compile("/foo<123>");
    BOOST_TEST(p);
    std::vector<nbt_path::pathspec> path = p.path();
    BOOST_TEST(path.size() == 2);
    BOOST_TEST(path[0].category() == nbt_path::pathspec::category::KEY);
    BOOST_TEST(path[0].container() == nbt_path::pathspec::container::NONE);
    BOOST_TEST(path[0].key() == "foo");
    BOOST_TEST(path[1].category() == nbt_path::pathspec::category::INDEX);
    BOOST_TEST(path[1].container() == nbt_path::pathspec::container::LIST);
    BOOST_TEST(path[1].index() == 123); // NOLINT
}

BOOST_AUTO_TEST_CASE(compound_1) {
    auto p = nbt_path::compile("/foo[1]/bar");
    BOOST_TEST(p);
    std::vector<nbt_path::pathspec> path = p.path();
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
    auto p = nbt_path::compile("/foo/<1>/");
    BOOST_TEST(p);
    std::vector<nbt_path::pathspec> path = p.path();
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
    auto p = nbt_path::compile("/foo//[1]//");
    BOOST_TEST(p);
    std::vector<nbt_path::pathspec> path = p.path();
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
    auto p = nbt_path::compile("/foo/bar/baz");
    BOOST_TEST(p);
    BOOST_TEST(p.remain());
    BOOST_TEST(p.get_one().key() == "foo");
    BOOST_TEST(p.remain());
    BOOST_TEST(p.get_one().key() == "bar");
    BOOST_TEST(p.remain());
    BOOST_TEST(p.get_one().key() == "baz");
    BOOST_TEST(!p.remain());
}

BOOST_AUTO_TEST_CASE(broken_1) {
    auto p = nbt_path::compile("foo/bar/baz");
    BOOST_TEST(!p);
}

BOOST_AUTO_TEST_CASE(broken_2) {
    auto p = nbt_path::compile("/foo[0");
    BOOST_TEST(!p);
}

BOOST_AUTO_TEST_CASE(broken_3) {
    auto p = nbt_path::compile("/foo<0");
    BOOST_TEST(!p);
}

BOOST_AUTO_TEST_CASE(broken_4) {
    auto p = nbt_path::compile("/foo[a]");
    BOOST_TEST(!p);
}

BOOST_AUTO_TEST_CASE(broken_5) {
    auto p = nbt_path::compile("/foo<a>");
    BOOST_TEST(!p);
}

BOOST_AUTO_TEST_CASE(broken_6) {
    auto p = nbt_path::compile("/foo[a");
    BOOST_TEST(!p);
}

BOOST_AUTO_TEST_CASE(broken_7) {
    auto p = nbt_path::compile("/foo<a");
    BOOST_TEST(!p);
}
