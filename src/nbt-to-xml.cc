// SPDX-License-Identifier: MIT

#include <cstring>
#include <exception>
#include <iostream>

#include <regetopt.h>

#include "config.h"

#include "nbt/file.hh"
#if USE_V3_NBT_PARSER
#include "nbt/tag.hh"
#else
#include "nbt/pull_parser/nbt_pull_parser.hh"
#endif
#include "pixel-terrain.hh"
#include "utils/array.hh"
#include "utils/path_hack.hh"

namespace {
    void print_usage() {
        std::cout << &R"(
Usage: pixel-terrain nbt-to-xml [OPTION]... [--] FILE

  -s STR, --indent STR  Use STR to indent. (default: "  ")
  -u, --no-prettify     Don't emit indent and new line.
      --help            Print this usage and exit.
)"[1];
    }

    std::string indent_str = "  ";
    bool pretty_print = true;

#if !USE_V3_NBT_PARSER
    auto get_tag_name(unsigned char tag_type) -> std::string {
        using namespace pixel_terrain;
        using namespace std::string_literals;

        switch (tag_type) {
        case nbt::TAG_END:
            /* should NOT occur. */
            return "End"s;

        case nbt::TAG_BYTE:
            return "Byte"s;

        case nbt::TAG_SHORT:
            return "Short"s;

        case nbt::TAG_INT:
            return "Int"s;

        case nbt::TAG_LONG:
            return "Long"s;

        case nbt::TAG_FLOAT:
            return "Float"s;

        case nbt::TAG_DOUBLE:
            return "Double"s;

        case nbt::TAG_BYTE_ARRAY:
            return "ByteArray"s;

        case nbt::TAG_STRING:
            return "String"s;

        case nbt::TAG_LIST:
            return "List"s;

        case nbt::TAG_COMPOUND:
            return "Compound"s;

        case nbt::TAG_INT_ARRAY:
            return "IntArray"s;

        case nbt::TAG_LONG_ARRAY:
            return "LongArray"s;

        default:
            return "unknown"s;
        }
    }
#endif

    auto handle_file(std::filesystem::path const &file) -> bool {
        using namespace pixel_terrain;

        pixel_terrain::file<unsigned char> *f;
        try {
            f = new pixel_terrain::file<unsigned char>(file);
        } catch (const std::exception &) {
            std::cerr << "Fatal: Unable to open input file.\n";
            return false;
        }

#if USE_V3_NBT_PARSER
        std::vector<std::uint8_t> data(f->get_raw_data(),
                                       f->get_raw_data() + f->size());
        auto [nbt, itr] =
            nbt::tag_compound::parse_buffer(data.begin(), data.end());
        if (nbt == nullptr) {
            std::cerr << "Fatal: Parse error in NBT.\n";
            return false;
        }

        std::cout << R"(<?xml version="1.0" encoding="utf-8"?>)";
        nbt->repr(std::cout, indent_str, 0);

        delete nbt;
#else
        unsigned char *data = f->get_raw_data();
        size_t size = f->size();

        nbt::nbt_pull_parser p(data, size);
        nbt::parser_event ev = p.get_event_type();
        int indent = 0;

        while (ev != nbt::parser_event::DOCUMENT_END) {
            switch (ev) {
            case nbt::parser_event::DOCUMENT_START:
                std::cout << R"(<?xml version="1.0" encoding="utf-8"?>)";
                if (pretty_print) {
                    std::cout << '\n';
                }
                break;

            case nbt::parser_event::TAG_START:
                if (pretty_print) {
                    for (int i = 0; i < indent; ++i) {
                        std::cout << indent_str;
                    }
                }
                std::cout << '<' << get_tag_name(p.get_tag_type());
                if (!p.get_tag_name().empty()) {
                    std::cout << " name=\"" << p.get_tag_name() << "\"";
                }
                std::cout << '>';
                if (pretty_print) {
                    std::cout << '\n';
                }

                ++indent;
                break;

            case nbt::parser_event::DATA:
                if (pretty_print) {
                    for (int i = 0; i < indent; ++i) {
                        std::cout << indent_str;
                    }
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

                if (pretty_print) {
                    std::cout << '\n';
                }
                break;

            case nbt::parser_event::TAG_END:
                --indent;
                if (pretty_print) {
                    for (int i = 0; i < indent; ++i) {
                        std::cout << indent_str;
                    }
                }
                std::cout << "</" << get_tag_name(p.get_tag_type()) << '>';
                if (pretty_print) {
                    std::cout << '\n';
                }
                break;

            default:
                break;
            }

            try {
                ev = p.next();
            } catch (const std::exception &e) {
                std::cerr << "Fatal: Broken NBT data: " << e.what() << '\n';
                delete f;

                return false;
            }
        }
#endif

        delete f;
        return true;
    }

    auto long_options = pixel_terrain::make_array<::re_option>(
        ::re_option{"indent", re_required_argument, nullptr, 's'},
        ::re_option{"no-prettify", re_no_argument, nullptr, 'u'},
        ::re_option{"help", re_no_argument, nullptr, 'h'},
        ::re_option{"version", re_no_argument, nullptr, 'v'},
        ::re_option{nullptr, 0, nullptr, 0});
} // namespace

namespace pixel_terrain {
    auto nbt_to_xml_main(int argc, char **argv) -> int {
        for (;;) {
            int opt = regetopt(argc, argv, "s:u", long_options.data(), nullptr);
            if (opt < 0) {
                break;
            }

            switch (opt) {
            case 's':
                indent_str = re_optarg;
                break;

            case 'u':
                pretty_print = false;
                break;

            case 'h':
                print_usage();
                return 0;

            default:
                return 1;
            }
        }
        if (argc - re_optind != 1) {
            print_usage();
            return 1;
        }

        return static_cast<int>(!handle_file(argv[re_optind]));
    }
} // namespace pixel_terrain
