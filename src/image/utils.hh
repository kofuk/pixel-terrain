// SPDX-Licnse-Identifier: MIT

#ifndef IMAGE_UTILS_HH
#define IMAGE_UTILS_HH

#include <filesystem>

#include "utils/path_hack.hh"

namespace pixel_terrain::image {
    auto make_output_name_by_input(std::filesystem::path const &in_name,
                                   std::filesystem::path const &output_dir)
        -> std::pair<path_string, bool>;

    auto format_output_name(std::string const &format, int x, int z)
        -> path_string;

    auto parse_region_file_path(
        std::filesystem::path const &file_path) noexcept(false)
        -> std::tuple<int, int, bool>;
} // namespace pixel_terrain::image

#endif
