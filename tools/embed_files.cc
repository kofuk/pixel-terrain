// SPDX-License-Identifier: MIT

#include <bits/c++config.h>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace {
    std::uint64_t header_id;

    class embedded_file {
        std::string filename_;
        std::filesystem::path in_path_;
        std::size_t file_size_;

    public:
        embedded_file(std::string const &filename,
                      std::filesystem::path const &in_path)
            : filename_(filename), in_path_(in_path) {}

        [[nodiscard]] auto filename() const -> std::string const & {
            return filename_;
        }

        [[nodiscard]] auto in_file_path() const
            -> std::filesystem::path const & {
            return in_path_;
        }

        [[nodiscard]] auto file_size() -> std::size_t & { return file_size_; }
    };

    void init() {
        std::random_device dev;
        std::mt19937_64 engine(dev());
        header_id = engine();
    }

    void write_header(std::FILE *out) {
        std::fputs("// DO NOT EDIT\n", out);
        std::fputs("// This is an auto-generated header.\n\n", out);
        std::fprintf(out, "#ifndef EMBEDDED_FILE_%lX\n", header_id);
        std::fprintf(out, "#define EMBEDDED_FILE_%lX 1\n", header_id);
        std::fputs(R"(
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace {
)",
                   out);
    }

    void write_footer(std::FILE *out) {
        std::fprintf(out, R"(
    auto get_embedded_data(std::string_view const &filename)
        -> std::optional<std::vector<std::uint8_t>> {
        auto itr = embedded_files_%1$lX.find(filename);
        if (itr == embedded_files_%1$lX.end()) {
            return std::nullopt;
        }
        auto [ptr, len] = itr->second;
        return std::vector<std::uint8_t>(ptr, ptr + len);
    }
} // namespace

)",
                     header_id);
        std::fprintf(out, "#endif /* EMBEDDED_FILE_%lX */\n", header_id);
    }

    auto write_files(std::vector<embedded_file> &in_files, std::FILE *out)
        -> bool {
        std::size_t file_no = 0;
        for (embedded_file &f : in_files) {
            fprintf(out, "    std::uint8_t FILE_%lX_%zu[] = { ", header_id,
                    file_no);

            std::ifstream in(f.in_file_path(), std::ios_base::binary);
            if (!in) {
                std::cerr << "Cannot open " + f.in_file_path().string() << '\n';
                return false;
            }

            std::uint8_t byte;
            std::size_t n_byte = 0;
            for (;;) {
                int byte = in.get();
                if (byte == EOF) {
                    break;
                }
                if (n_byte % 16 == 0) {
                    std::fputs("// NOLINT\n        ", out);
                }
                std::fprintf(out, "%u, ", byte);

                ++n_byte;
            }
            if (!in.eof()) {
                std::cerr << "Input failed befare reaching EOF.";
                return false;
            }
            std::fputs("// NOLINT\n    };\n", out);

            ++file_no;
            f.file_size() = n_byte;
        }

        return true;
    }

    void write_file_mapping(std::vector<embedded_file> &in_files,
                            std::FILE *out) {
        std::fprintf(
            out,
            R"(
    std::unordered_map<std::string_view, std::pair<std::uint8_t *, std::size_t>>
        embedded_files_%lX = {
)",
            header_id);

        std::size_t file_no = 0;
        for (embedded_file &f : in_files) {
            std::fprintf(out,
                         R"(        {"%s", std::make_pair(FILE_%lX_%zu, %zuUL)},)"
                         "\n",
                         f.filename().c_str(), header_id, file_no,
                         f.file_size());

            ++file_no;
        }
        std::fputs("  };\n", out);
    }
} // namespace

auto main(int argc, char **argv) -> int {
    if (argc < 3) {
        std::cerr << "Usage: embed_files <out file> [file]...";
        return 1;
    }

    init();

    std::FILE *out = std::fopen(argv[1], "w");
    if (out == NULL) {
        std::cerr << "Cannot open output file\n";
        return 1;
    }

    write_header(out);

    std::vector<embedded_file> in_files;
    for (int i = 2; i < argc; ++i) {
        std::filesystem::path in_file(argv[i]);
        if (!std::filesystem::is_regular_file(in_file)) {
            std::cerr << "Cannot embed " + in_file.string() << '\n';
            return 1;
        }
        in_files.emplace_back(in_file.filename().string(), in_file);
    }

    if (!write_files(in_files, out)) {
        return 1;
    }

    write_file_mapping(in_files, out);

    write_footer(out);
}
