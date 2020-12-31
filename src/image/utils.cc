// SPDX-License-Identifier: MIT

#include <filesystem>
#include <tuple>
#include <vector>

#include "utils/path_hack.hh"

namespace pixel_terrain::image {
    auto make_output_name_by_input(std::filesystem::path const &in_path,
                                   std::filesystem::path const &output_dir)
        -> std::pair<path_string, bool> {
        if (in_path.extension() != PATH_STR_LITERAL(".mca") ||
            !in_path.has_filename()) {
            return std::make_pair(std::filesystem::path(), false);
        }

        std::filesystem::path in_name_wo_ext(
            in_path.filename().generic_string().substr(
                0, in_path.filename().generic_string().size() - 4));

        if (in_name_wo_ext.generic_string().empty()) {
            return std::make_pair(std::filesystem::path(), false);
        }

        return std::make_pair(
            (output_dir / in_name_wo_ext).concat(PATH_STR_LITERAL(".png")),
            true);
    }

    auto format_output_name(std::string const &format, int x, int z)
        -> path_string {
        path_string x_str = to_path_string(x);
        path_string z_str = to_path_string(z);
        path_string result;

        bool is_format = false;
        for (path_char const &c : format) {
            if (!is_format && c == PATH_STR_LITERAL('%')) {
                is_format = true;
                continue;
            }

            if (is_format) {
                switch (c) {
                case PATH_STR_LITERAL('X'):
                    result.insert(result.end(), x_str.begin(), x_str.end());
                    break;

                case PATH_STR_LITERAL('Z'):
                    result.insert(result.end(), z_str.begin(), z_str.end());
                    break;

                case PATH_STR_LITERAL('%'):
                    result.push_back(PATH_STR_LITERAL('%'));
                    break;

                default:
                    result.push_back(PATH_STR_LITERAL('%'));
                    result.push_back(c);
                    break;
                }

                is_format = false;
            } else {
                result.push_back(c);
            }
        }
        if (is_format) {
            result.push_back(PATH_STR_LITERAL('%'));
        }

        return result;
    }

    auto parse_region_file_path(
        std::filesystem::path const &file_path) noexcept(false)
        -> std::tuple<int, int, bool> {
        std::string filename = file_path.filename().string();
        if (filename.empty()) {
            return std::make_tuple(0, 0, false);
        }

        std::vector<std::string> elements;
        std::size_t prev = 0;
        for (std::size_t i = 0; i < filename.size(); ++i) {
            if (filename[i] == '.') {
                elements.push_back(filename.substr(prev, i - prev));
                prev = i + 1;
            }
        }
        if (prev <= filename.size()) {
            elements.push_back(filename.substr(prev));
        }

        if (elements.size() != 4 || elements[0] != "r" ||
            elements[3] != "mca") {
            return std::make_tuple(0, 0, false);
        }

        std::size_t idx;
        int x = std::stoi(elements[1], &idx);
        if (idx != elements[1].size()) {
            return std::make_tuple(0, 0, false);
        }
        int z = std::stoi(elements[2], &idx);
        if (idx != elements[2].size()) {
            return std::make_tuple(0, 0, false);
        }

        return std::make_tuple(x, z, true);
    }
} // namespace pixel_terrain::image
