// SPDX-Licnse-Identifier: MIT

#ifndef IMAGE_UTILS_HH
#define IMAGE_UTILS_HH

#include <filesystem>

#include "utils/path_hack.hh"

namespace pixel_terrain::image {
    std::pair<path_string, bool>
    make_output_name_by_input(std::filesystem::path const &in_name,
                              std::filesystem::path const &output_dir);

    path_string format_output_name(std::string const &format, int x, int z);

    std::tuple<int, int, bool> parse_region_file_path(
        std::filesystem::path const &file_path) noexcept(false);
} // namespace pixel_terrain::image

#endif
