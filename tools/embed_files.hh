// SPDX-License-Identifier: MIT

#ifndef EMBED_FILES_HH
#define EMBED_FILES_HH

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

class embedded_file {
    std::string filename_;
    std::vector<std::uint8_t> data_;

public:
    embedded_file(std::string const &filename, std::vector<std::uint8_t> data)
        : filename_(filename), data_(std::move(data)) {}

    [[nodiscard]] auto filename() const -> std::string const & {
        return filename_;
    }

    [[nodiscard]] auto data() const -> std::vector<std::uint8_t> const & {
        return data_;
    }

    [[nodiscard]] auto file_size() const -> std::size_t { return data_.size(); }
};

auto embed_files(std::filesystem::path const &out_path,
                 std::vector<embedded_file> files) -> bool;

#endif
