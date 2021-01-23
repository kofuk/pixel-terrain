// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <streambuf>
#include <tuple>
#include <vector>

namespace {
    inline constexpr std::size_t BUF_SIZE = 1024;
    using data_iterator = std::string::const_iterator;

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

            switch (*itr) {
            case '0':
                result = '\0';
                break;
            case 'e':
                result = '\e';
                break;
            case 'f':
                result = '\f';
                break;
            case 'n':
                result = '\n';
                break;
            case 'r':
                result = '\r';
                break;
            case 't':
                result = '\t';
                break;
            case 'v':
                result = '\v';
                break;
            default:
                result = *itr;
            }
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
            } else if (*itr == ' ' || *itr == '\t' || *itr == '\v' ||
                       *itr == '\r' || *itr == '\n') {
                continue;
            } else if (*itr == '#') {
                is_in_comment = true;
                continue;
            }

            return itr;
        }

        return last;
    }

    [[nodiscard]] auto write_data(std::vector<std::uint8_t> const &bytes,
                                  std::FILE *to) -> bool {
        return std::fwrite(bytes.data(), 1, bytes.size(), to) == bytes.size();
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

    auto parse_and_write(std::string const &source, std::FILE *out) -> bool {
        std::vector<std::uint8_t> bytes;

        data_iterator itr = skip_space(source.cbegin(), source.cend());
        data_iterator last = source.cend();
        for (;;) {
            auto [it, result] = read_entry(itr, last);
            if (!result) {
                if (it == last) {
                    /* Allow a trailing comma. */
                    return write_data(bytes, out);
                } else {
                    print_diag(source, it, "Syntax error.");
                    return false;
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
                        return false;
                    }
                }
                itr = skip_space(it, last);
            }
        }

        return write_data(bytes, out);
    }
} // namespace

auto main(int argc, char **argv) -> int {
    if (argc < 3) {
        std::cout << "Usage: generate_binary SRC DST\n";
        return 1;
    }

    std::ifstream infile(argv[1]);
    std::FILE *outfile = std::fopen(argv[2], "wb");
    if (!infile || outfile == nullptr) {
        std::cerr << "generate_binary: Cannot open input or output file.\n";
        return 1;
    }

    std::string data((std::istreambuf_iterator<char>(infile)),
                     std::istreambuf_iterator<char>());

    if (!parse_and_write(data, outfile)) {
        return 1;
    }
}
