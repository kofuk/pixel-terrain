// SPDX-License-Identifier: MIT

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#ifdef TEST
#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#endif

namespace {
    void print_usage() {
        std::cout << R"(usage: lsp-aux TYPE OUT_FILE ARGS...
TYPEs:
  compile_flags.txt
  .dir-local.el
)";
    }

    void generate_compile_flags(std::vector<std::string> const &flags,
                                std::string const &out) {
        std::ofstream of(out);
        for (std::string const &flag : flags) {
            of << flag << '\n';
        }
    }

    inline void escape_string(std::string &str) {
        for (auto itr = str.begin(); itr != str.end(); ++itr) {
            if (*itr == '"') {
                str.insert(itr, '\\');
                ++itr;
            }
        }
    }

    void escape_strings(std::vector<std::string> &strs) {
        for (std::string &str : strs) {
            escape_string(str);
        }
    }

    void generate_dir_locals(std::vector<std::string> const &include_dirs,
                             std::string const &out) {
        std::ofstream of(out);
        of << "((c++-mode . ((flycheck-clang-include-path . (list ";
        for (std::string const &path : include_dirs) {
            of << "\"" << path << "\" ";
        }
        of << ")))))\n";
    }
} // namespace

#ifndef TEST
int main(int argc, char **argv) {
    if (argc < 3) {
        print_usage();
        return 1;
    }
    if (!strcmp(argv[1], "compile_flags.txt")) {
        std::vector<std::string> flags;
        flags.push_back("-Wall");
        flags.push_back("-Wextra");
        flags.push_back("-std=c++17");
        for (int i = 3; i < argc; ++i) {
            using namespace std::string_literals;
            flags.push_back("-I"s + argv[i]);
        }
        generate_compile_flags(flags, argv[2]);
    } else if (!strcmp(argv[1], ".dir-locals.el")) {
        std::vector<std::string> include_dirs;
        for (int i = 3; i < argc; ++i) {
            include_dirs.push_back(argv[i]);
        }
        escape_strings(include_dirs);
        generate_dir_locals(include_dirs, argv[2]);
    } else {
        print_usage();
        return 1;
    }
}
#else
BOOST_AUTO_TEST_CASE(escape_string_test) {
    std::string test_str = "foo";
    escape_string(test_str);
    BOOST_TEST(test_str == "foo");

    test_str = "\"";
    escape_string(test_str);
    BOOST_TEST(test_str == "\\\"");

    test_str = "foo\"bar";
    escape_string(test_str);
    BOOST_TEST(test_str == "foo\\\"bar");

    test_str = "\"foo";
    escape_string(test_str);
    BOOST_TEST(test_str == "\\\"foo");

    test_str = "foo\"";
    escape_string(test_str);
    BOOST_TEST(test_str == "foo\\\"");
}
#endif
