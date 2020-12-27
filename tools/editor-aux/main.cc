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
        std::cout << &R"(
usage: lsp-aux PLATFORM TYPE OUT_FILE ARGS...
Platforms:
  Linux
  Windows
Types:
  compile_flags.txt
  .dir-local.el
)"[1];
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

    void
    generate_dir_locals(std::vector<std::string> const &include_dirs,
                        std::vector<std::string> const &compile_definitions,
                        std::string const &out) {
        std::ofstream of(out);
        of << "((c++-mode . ((flycheck-clang-include-path . (list ";
        for (std::string const &path : include_dirs) {
            of << "\"" << path << "\" ";
        }
        of << "))\n(flycheck-clang-definitions . (list ";
        for (std::string const &def : compile_definitions) {
            of << "\"" << def << "\" ";
        }
        of << ")))))\n";
    }
} // namespace

#ifndef TEST
int main(int argc, char **argv) {
    if (argc < 4) {
        print_usage();
        return 1;
    }

    if (!strcmp(argv[2], "compile_flags.txt")) {
        std::vector<std::string> flags;
        flags.push_back("-Wall");
        flags.push_back("-Wextra");
        flags.push_back("-std=c++17");

        if (!std::strcmp(argv[1], "Linux")) {
            flags.push_back("-DOS_LINUX=1");
        } else if (!strcmp(argv[1], "Windows")) {
            flags.push_back("-DOS_WIN=1");
        } else {
            print_usage();
            return 1;
        }

        for (int i = 4; i < argc; ++i) {
            using namespace std::string_literals;
            flags.push_back("-I"s + argv[i]);
        }
        generate_compile_flags(flags, argv[3]);
    } else if (!strcmp(argv[2], ".dir-locals.el")) {
        std::vector<std::string> include_dirs;
        std::vector<std::string> compile_definitions;
        if (!std::strcmp(argv[1], "Linux")) {
            compile_definitions.push_back("-DOS_LINUX=1");
        } else if (!std::strcmp(argv[1], "Windows")) {
            compile_definitions.push_back("-DOS_WIN=1");
        } else {
            print_usage();
            return 1;
        }

        for (int i = 4; i < argc; ++i) {
            include_dirs.push_back(argv[i]);
        }
        escape_strings(include_dirs);
        generate_dir_locals(include_dirs, compile_definitions, argv[3]);
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
