#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#ifdef TEST
#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#endif

namespace {
    void print_usage() {
        std::cout << "usage: lsp-aux TYPE OUT_FILE ARGS..." << std::endl;
        std::cout << "TYPEs:" << std::endl;
        std::cout << "  compile_flags.txt" << std::endl;
        std::cout << "  .dir-local.el" << std::endl;
    }

    void generate_compile_flags(int count, char **args,
                                std::string const &out) {
        std::ofstream of(out);
        of << "-Wall" << std::endl;
        of << "-Wextra" << std::endl;
        of << "-std=c++17" << std::endl;
        for (int i = 0; i < count; ++i) {
            of << "-I" << args[i] << std::endl;
        }
    }

    void escape_string(std::string &str) {
        for (auto itr = str.begin(); itr != str.end(); ++itr) {
            if (*itr == '"') {
                str.insert(itr, '\\');
                ++itr;
            }
        }
    }

    void generate_dir_locals(int count, char **args, std::string const &out) {
        std::ofstream of(out);
        of << "((c++-mode . ((flycheck-clang-include-path . (list ";
        for (int i = 0; i < count; ++i) {
            std::string cur_dir = args[i];
            escape_string(cur_dir);
            of << "\"" << cur_dir << "\" ";
        }
        of << ")))))" << std::endl;
    }
} // namespace

#ifndef TEST
int main(int argc, char **argv) {
    if (argc < 3) {
        print_usage();
        return 1;
    }
    if (!strcmp(argv[1], "compile_flags.txt")) {
        generate_compile_flags(argc - 3, argv + 3, argv[2]);
    } else if (!strcmp(argv[1], ".dir-locals.el")) {
        generate_dir_locals(argc - 3, argv + 3, argv[2]);
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
