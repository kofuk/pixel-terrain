// SPDX-License-Identifier: MIT

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "embed_files.hh"

namespace {
    void write_header(std::FILE *out, std::uint64_t header_id) {
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

    void write_footer(std::FILE *out, std::uint64_t header_id) {
        std::fprintf(out, R"(
    auto get_embedded_data(std::string_view const &filename)
        -> std::optional<std::vector<std::uint8_t>> {
        auto itr = embedded_files_%lX.find(filename);
        if (itr == embedded_files_%lX.end()) {
            return std::nullopt;
        }
        auto [ptr, len] = itr->second;
        return std::vector<std::uint8_t>(ptr, ptr + len);
    }
} // namespace

)",
                     header_id, header_id);
        std::fprintf(out, "#endif /* EMBEDDED_FILE_%lX */\n", header_id);
    }

    auto write_files(std::vector<embedded_file> &in_files, std::FILE *out,
                     std::uint64_t header_id) -> bool {
        std::size_t file_no = 0;
        for (embedded_file &f : in_files) {
            fprintf(out, "    std::uint8_t FILE_%lX_%zu[] = { ", header_id,
                    file_no);

            std::size_t n_byte = 0;
            for (std::uint8_t const b : f.data()) {
                if (n_byte % 16 == 0) {
                    std::fputs("// NOLINT\n        ", out);
                }
                std::fprintf(out, "%u, ", b);

                ++n_byte;
            }
            std::fputs("// NOLINT\n    };\n", out);

            ++file_no;
        }

        return true;
    }

    void write_file_mapping(std::vector<embedded_file> &in_files,
                            std::FILE *out, std::uint64_t header_id) {
        std::fprintf(out,
                     R"(
    std::unordered_map<std::string_view, std::pair<std::uint8_t *, std::size_t>>
        embedded_files_%lX = {
)",
                     header_id);

        std::size_t file_no = 0;
        for (embedded_file &f : in_files) {
            std::fprintf(
                out,
                R"(        {"%s", std::make_pair(FILE_%lX_%zu, %zuUL)},)"
                "\n",
                f.filename().c_str(), header_id, file_no, f.file_size());

            ++file_no;
        }
        std::fputs("  };\n", out);
    }
} // namespace

auto embed_files(std::filesystem::path const &out_path,
                 std::vector<embedded_file> files) -> bool {
    std::random_device dev;
    std::mt19937_64 engine(dev());
    std::uint64_t header_id = engine();

    std::FILE *out = std::fopen(out_path.string().c_str(), "wb");
    if (out == NULL) {
        std::cerr << "Cannot open output file\n";
        return false;
    }

    write_header(out, header_id);

    if (!write_files(files, out, header_id)) {
        return false;
    }

    write_file_mapping(files, out, header_id);
    write_footer(out, header_id);

    std::fclose(out);

    return true;
}
