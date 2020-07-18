/*
 * Copyright (c) 2020 Koki Fukuda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
