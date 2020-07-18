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
#include <exception>
#include <iostream>

#include "nbt/file.hh"
#include "nbt/pull_parser/nbt_pull_parser.hh"
#include "utils/path_hack.hh"
#include "version.hh"

namespace {
    void print_usage() { std::cout << "usage: nbt2xml nbt_file\n"; }

    void print_version() {
        std::cout << "nbt2xml (" PROJECT_NAME " " VERSION_MAJOR
                     "." VERSION_MINOR "." VERSION_REVISION ")\n";
        std::cout << R"(
Copyright (C) 2020  Koki Fukuda.
Visit https://github.com/kofuk/minecraft-image-gemerator for the source code.
)";
    }

    bool handle_file(const std::filesystem::path &file) {
        using namespace pixel_terrain;

        pixel_terrain::file<unsigned char> f(file);
        unsigned char *data = f.get_raw_data();
        size_t size = f.size();

        nbt::nbt_pull_parser p(data, size);
        nbt::parser_event ev = p.get_event_type();
        int indent = 0;

        while (ev != nbt::parser_event::DOCUMENT_END) {
            switch (ev) {
            case nbt::parser_event::DOCUMENT_START:
                std::cout << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
                break;

            case nbt::parser_event::TAG_START:
                for (int i = 0; i < indent; ++i) {
                    std::cout << ' ';
                }
                switch (p.get_tag_type()) {
                case nbt::TAG_BYTE:
                    std::cout << "<Byte";
                    break;

                case nbt::TAG_SHORT:
                    std::cout << "<Short";
                    break;

                case nbt::TAG_INT:
                    std::cout << "<Int";
                    break;

                case nbt::TAG_LONG:
                    std::cout << "<Long";
                    break;

                case nbt::TAG_FLOAT:
                    std::cout << "<Float";
                    break;

                case nbt::TAG_DOUBLE:
                    std::cout << "<Double";
                    break;

                case nbt::TAG_BYTE_ARRAY:
                    std::cout << "<ByteArray";
                    break;

                case nbt::TAG_STRING:
                    std::cout << "<String";
                    break;

                case nbt::TAG_LIST:
                    std::cout << "<List";
                    break;

                case nbt::TAG_INT_ARRAY:
                    std::cout << "<IntArray";
                    break;

                case nbt::TAG_LONG_ARRAY:
                    std::cout << "<LongArray";
                    break;

                case nbt::TAG_COMPOUND:
                    std::cout << "<Compound";
                    break;
                }

                if (!p.get_tag_name().empty()) {
                    std::cout << " name=\"" << p.get_tag_name() << "\"";
                }
                std::cout << ">\n";

                indent += 2;
                break;

            case nbt::parser_event::DATA:
                for (int i = 0; i < indent; ++i) {
                    std::cout << ' ';
                }
                switch (p.get_tag_type()) {
                case nbt::TAG_BYTE:
                    std::cout << +p.get_byte();
                    break;

                case nbt::TAG_SHORT:
                    std::cout << p.get_short();
                    break;

                case nbt::TAG_INT:
                    std::cout << p.get_int();
                    break;

                case nbt::TAG_LONG:
                    std::cout << p.get_long();
                    break;

                case nbt::TAG_FLOAT:
                    std::cout << p.get_float();
                    break;

                case nbt::TAG_DOUBLE:
                    std::cout << p.get_double();
                    break;

                case nbt::TAG_BYTE_ARRAY:
                    std::cout << "<item>" << +p.get_byte() << "</item>";
                    break;

                case nbt::TAG_STRING:
                    std::cout << p.get_string();
                    break;

                case nbt::TAG_INT_ARRAY:
                    std::cout << "<item>" << p.get_int() << "</item>";
                    break;

                case nbt::TAG_LONG_ARRAY:
                    std::cout << "<item>" << p.get_long() << "</item>";
                }
                std::cout << '\n';
                break;

            case nbt::parser_event::TAG_END:
                indent -= 2;
                for (int i = 0; i < indent; ++i) {
                    std::cout << ' ';
                }
                switch (p.get_tag_type()) {
                case nbt::TAG_BYTE:
                    std::cout << "</Byte>";
                    break;

                case nbt::TAG_SHORT:
                    std::cout << "</Short>";
                    break;

                case nbt::TAG_INT:
                    std::cout << "</Int>";
                    break;

                case nbt::TAG_LONG:
                    std::cout << "</Long>";
                    break;

                case nbt::TAG_FLOAT:
                    std::cout << "</Float>";
                    break;

                case nbt::TAG_DOUBLE:
                    std::cout << "</Double>";
                    break;

                case nbt::TAG_BYTE_ARRAY:
                    std::cout << "</ByteArray>";
                    break;

                case nbt::TAG_STRING:
                    std::cout << "</String>";
                    break;

                case nbt::TAG_LIST:
                    std::cout << "</List>";
                    break;

                case nbt::TAG_INT_ARRAY:
                    std::cout << "</IntArray>";
                    break;

                case nbt::TAG_LONG_ARRAY:
                    std::cout << "</LongArray>";
                    break;

                case nbt::TAG_COMPOUND:
                    std::cout << "</Compound>";
                    break;
                }
                std::cout << '\n';

            default:
                break;
            }

            try {
                ev = p.next();
            } catch (const std::exception &e) {
                std::cerr << "Fatal: Broken NBT data: " << e.what() << '\n';
                return false;
            }
        }
        return true;
    }
} // namespace

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    if (!strcmp(argv[1], "--help")) {
        print_usage();
        return 0;
    } else if (!strcmp(argv[1], "--version")) {
        print_version();
        return 0;
    }

    return !handle_file(argv[1]);
}
