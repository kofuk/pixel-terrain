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
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef TEST
#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#endif

/*
 * DATA STRUCTURE
 *
 * |-------------------|-------------------------|
 * | block_id (string) | block id (byte by byte) |
 * | .                 |                         |
 * | .                 |                         |
 * | .                 |                         |
 * | 0                 | sentinel value          |
 * | color[0]          |                         |
 * | color[1]          |                         |
 * | color[2]          |                         |
 * | color[3]          |                         |
 * |-------------------+-------------------------|
 * |                   |                         |
 * | .                 |                         |
 * | .                 |                         |
 * | .                 |                         |
 * |                   |                         |
 * |-------------------+-------------------------|
 * | 0                 | sentinel value          |
 * |-------------------+-------------------------|
 *
 * order of color 0--3 is byte-order-dependent.
 * this program puts the bytes to construct 32 bits of int
 * represents RRGGBBAA.
 */

namespace {
    size_t total_elements;
    size_t total_bytes;

    void print_usage() {
        std::cout << "usage: block_data_generator outfile infile..."
                  << std::endl;
    }

    void write_preamble(std::ostream &out_file) {
        out_file << "/* DO NOT EDIT" << std::endl;
        out_file << "   This file has been automatically generated by "
                    "block_data_generator. */"
                 << std::endl;
        out_file << "#ifdef EMBEDED_BLOCK_COLOR_DATA_H" << std::endl;
        out_file << "#  error block color data has already embeded."
                 << std::endl;
        out_file << "#else" << std::endl;
        out_file << std::endl;
        out_file << "static unsigned char block_colors_data[] = {" << std::endl;
    }

    void write_postamble(std::ostream &out_file) {
        out_file << "0};" << std::endl << std::endl;
        ++total_bytes;

        out_file << "/* " << total_bytes << " bytes, " << total_elements
                 << " elements written. */" << std::endl;

        out_file << "#endif" << std::endl;
    }

    std::vector<std::string> split_field(std::string const &record) {
        std::vector<std::string> result;
        size_t beg = 0;
        for (size_t i = 0; i < record.size(); ++i) {
            if (record[i] == '\t') {
                result.push_back(record.substr(beg, i - beg));
                beg = i + 1;
            }
        }
        result.push_back(record.substr(beg, record.size() - beg));
        return result;
    }

    bool write_content(std::string in_name, std::ostream &out_file) {
        std::ifstream in_file(in_name);
        if (!in_file) {
            std::cerr << "fatal: cannot open " << in_name << std::endl;
            return false;
        }

        std::string line;
        if (!std::getline(in_file, line)) {
            std::cerr << "fatal: empty file: " << in_name << std::endl;
            return false;
        }
        size_t lineno = 2;
        for (; std::getline(in_file, line); ++lineno) {
            std::vector<std::string> fields = split_field(line);
            if (fields.size() != 5) {
                std::cerr << "fatal: invalid line: " << in_name << ": "
                          << lineno << std::endl;
                return false;
            }
            std::string namespace_id = fields[0];
            for (size_t i = 0; i < namespace_id.size(); ++i) {
                out_file << '\'' << namespace_id[i] << '\'';
                out_file << ',';
            }
            out_file << '0' << ',';
            total_bytes += fields[0].size() + 1;

            unsigned char color_components[4];

            for (int c = 0; c < 4; ++c) {
                try {
                    int color = std::stoi(fields[c + 1], nullptr, 16);
                    if (color < 0 || 255 < color) {
                        std::cerr
                            << "fatal: invalid color component (out of range): "
                            << in_name << ": " << lineno << std::endl;
                        return false;
                    }
                    color_components[c] = color;
                } catch (std::invalid_argument const &e) {
                    std::cerr << "fatal: invalid color component: " << in_name
                              << ": " << lineno << std::endl;
                    return false;
                }
            }

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            for (int i = 3; i >= 0; --i) {
                out_file << +color_components[i] << ',';
            }
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            for (int i = 0; i < 4; ++i) {
                out_file << +color_components[i] << ',';
            }
#else
#error "Unsupported byte order"
#endif

            total_bytes += 4;
            ++total_elements;

            out_file << std::endl;
        }
        return true;
    }
} // namespace

#ifndef TEST
int main(int argc, char **argv) {
    if (argc < 2 || !strcmp(argv[1], "--help")) {
        print_usage();
        return 1;
    }
    char *out_name = argv[1];
    std::ofstream out_file(out_name);
    if (!out_file) {
        std::cerr << "fatal: cannot open " << out_name << std::endl;
        return 1;
    }
    write_preamble(out_file);
    for (int i = 2; i < argc; ++i) {
        if (!write_content(argv[i], out_file)) {
            std::cerr << "fatal: cannot process " << argv[i] << std::endl;
            return 1;
        }
    }
    write_postamble(out_file);
}
#endif

#ifdef TEST
BOOST_AUTO_TEST_CASE(split_field_test) {
    std::vector<std::string> fields = split_field("foo\tbar\tbaz");
    BOOST_TEST(fields.size() == 3);
    BOOST_TEST(fields[0] == "foo");
    BOOST_TEST(fields[1] == "bar");
    BOOST_TEST(fields[2] == "baz");

    fields = split_field("\tfoo\tbar\tbaz\t");
    BOOST_TEST(fields.size() == 5);
    BOOST_TEST(fields[0] == "");
    BOOST_TEST(fields[1] == "foo");
    BOOST_TEST(fields[2] == "bar");
    BOOST_TEST(fields[3] == "baz");
    BOOST_TEST(fields[4] == "");
}
#endif
