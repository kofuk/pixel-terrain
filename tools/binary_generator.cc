// SPDX-License-Identifier: MIT

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <ranges>
#include <streambuf>
#include <tuple>
#include <vector>

#include "embed_files.hh"

namespace {
    inline constexpr std::size_t BUF_SIZE = 1024;
    using data_iterator = std::string::const_iterator;

    // clang-format off
    std::array<int, 256> space_table{
        /*       x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xa xb xc xd xe xf*/
        /* 0x */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0,
        /* 1x */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 2x */ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 3x */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 4x */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 5x */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 6x */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 7x */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 8x */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 9x */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* ax */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* bx */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* cx */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* dx */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* ex */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* fx */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    std::array<std::uint8_t, 256> esc_map{
        /*       x0    x1    x2    x3    x4    x5    x6    x7    x8    x9    xa    xb    xc    xd    xe    xf */
        /* 0x */ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        /* 1x */ 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        /* 2x */ 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        /* 3x */ '\0', 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        /* 4x */ 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
        /* 5x */ 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, '\\', 0x5d, 0x5e, 0x5f,
        /* 6x */ 0x60, '\a', 0x62, 0x63, 0x64, 0x1b, '\f', 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, '\n', 0x6f,
        /* 7x */ 0x70, 0x71, '\r', 0x73, '\t', 0x75, '\v', 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
        /* 8x */ 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
        /* 9x */ 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
        /* ax */ 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
        /* bx */ 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
        /* cx */ 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
        /* dx */ 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
        /* ex */ 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
        /* fx */ 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    };
    // clang-format on

    class byte_builder {
        unsigned int num_ = 0;
        int base_;

    public:
        byte_builder(int base) : base_(base) {}

        [[nodiscard]] auto result() const -> std::uint8_t { return num_; }

        [[nodiscard]] auto append(std::uint8_t digit) -> bool {
            if (digit >= base_ || num_ * base_ + digit > 255) {
                return false;
            }
            num_ *= base_;
            num_ += digit;
            return true;
        }
    };

    [[nodiscard]] auto read_char_literal_entry(data_iterator const &first,
                                               data_iterator const &last)
        -> std::tuple<data_iterator, std::optional<std::uint8_t>> {
        if (last <= first + 3) {
            return std::make_tuple(first, std::nullopt);
        }

        data_iterator itr = first + 1;
        std::uint8_t result;
        if (*itr == '\\') {
            if (last <= first + 3) {
                return std::make_tuple(first, std::nullopt);
            }
            ++itr;

            result = esc_map[static_cast<std::size_t>(*itr)];
        } else {
            result = *itr;
        }

        ++itr;
        if (*itr != '\'') {
            return std::make_tuple(first, std::nullopt);
        }

        ++itr;
        return std::make_tuple(itr, result);
    }

    [[nodiscard]] auto read_entry(data_iterator const &first,
                                  data_iterator const &last)
        -> std::tuple<data_iterator, std::optional<std::uint8_t>> {
        if (last <= first) {
            return std::make_tuple(first, std::nullopt);
        }

        data_iterator itr = first;
        if (*itr == '\'') {
            return read_char_literal_entry(first, last);
        }
        int base = 10;
        if (itr + 2 < last) {
            if (*itr == '0') {
                base = 8;
                if (*(itr + 1) == 'x') {
                    itr += 2;
                    base = 16;
                } else if (*(itr + 1) == 'b') {
                    itr += 2;
                    base = 2;
                }
            }
        }

        data_iterator num_start = itr;

        byte_builder builder(base);
        for (; itr != last; ++itr) {
            std::uint8_t cur_digit;
            char cur = *itr;
            if ('0' <= cur && cur <= '9') {
                cur_digit = cur - '0';
            } else if ('a' <= cur && cur <= 'f') {
                cur_digit = 10 + cur - 'a';
            } else if ('A' <= cur && cur <= 'F') {
                cur_digit = 10 + cur - 'A';
            } else {
                break;
            }

            if (!builder.append(cur_digit)) {
                return std::make_tuple(first, std::nullopt);
            }
        }

        /* If no character was read, this is an error. */
        if (itr == num_start) {
            return std::make_tuple(first, std::nullopt);
        }

        return std::make_tuple(itr, builder.result());
    }

    [[nodiscard]] auto skip_space(data_iterator const &first,
                                  data_iterator const &last) -> data_iterator {
        bool is_in_comment = false;
        for (data_iterator itr = first; itr != last; ++itr) {
            if (is_in_comment) {
                if (*itr == '\n') {
                    is_in_comment = false;
                }
                continue;
            } else if (static_cast<bool>(
                           space_table[static_cast<std::size_t>(*itr)])) {
                continue;
            } else if (*itr == '#') {
                is_in_comment = true;
                continue;
            }

            return itr;
        }

        return last;
    }

    [[nodiscard]] auto read_entry_sep(data_iterator const &first,
                                      data_iterator const &last)
        -> std::tuple<data_iterator, bool> {
        if (last <= first) {
            return std::make_tuple(last, false);
        }
        if (*first == ',') {
            return std::make_tuple(first + 1, true);
        }
        return std::make_tuple(first, false);
    }

    void print_diag(std::string const &source, data_iterator const &place,
                    std::string const &message) {
        if (source.end() <= place) {
            std::cerr << "Unknown source location: " << message << '\n';
            return;
        }

        data_iterator line_start = source.begin();
        data_iterator line_end = std::find(source.begin(), source.end(), '\n');
        std::size_t line_no = 1;
        while (place < line_start || line_end < place) {
            line_start = line_end + 1;
            line_end = std::find(line_start, source.end(), '\n');
            ++line_no;
        }

        std::string line(line_start, line_end);
        std::size_t col = place - line_start + 1;

        std::cerr << "Error: " << line_no << ": " << col << ": " << message
                  << '\n';
        std::cerr << std::setw(5) << std::setfill(' ') << line_no << " | ";
        std::cerr << line << '\n';
        for (std::size_t i = 0; i < col + 7; ++i) {
            std::cerr << ' ';
        }
        std::cerr << "^\n";
    }

    auto parse(std::string const &source)
        -> std::optional<std::vector<std::uint8_t>> {
        std::vector<std::uint8_t> bytes;

        data_iterator itr = skip_space(source.cbegin(), source.cend());
        data_iterator last = source.cend();
        for (;;) {
            auto [it, result] = read_entry(itr, last);
            if (!result) {
                if (it == last) {
                    /* Allow a trailing comma. */
                    break;
                } else {
                    print_diag(source, it, "Syntax error.");
                    return std::nullopt;
                }
            }
            bytes.push_back(*result);
            itr = skip_space(it, last);
            {
                auto [it, result] = read_entry_sep(itr, last);
                if (!result) {
                    if (it == last) {
                        /* input terminated. */
                        break;
                    } else {
                        print_diag(source, it, "Syntax error.");
                        return std::nullopt;
                    }
                }
                itr = skip_space(it, last);
            }
        }

        return bytes;
    }
} // namespace

auto main(int argc, char **argv) -> int {
    if (argc < 2) {
        std::cout << "Usage: generate_binary <out> [src]...\n";
        return 1;
    }

    std::vector<embedded_file> files;
    for (int i = 2; i < argc; ++i) {
        std::filesystem::path in_path(argv[i]);
        std::ifstream infile(in_path);
        std::string source((std::istreambuf_iterator<char>(infile)),
                           std::istreambuf_iterator<char>());

        std::optional<std::vector<std::uint8_t>> bin = parse(source);
        if (!bin) {
            return 1;
        }

        files.emplace_back(in_path.filename(), *bin);
    }

    return embed_files(argv[1], files) ? 0 : 1;
}
