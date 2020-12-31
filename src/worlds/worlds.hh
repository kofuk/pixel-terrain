// SPDX-License-Identifier: MIT

#ifndef WORLDS_HH
#define WORLDS_HH

#include <filesystem>
#include <string>
#include <vector>

namespace pixel_terrain {
    class world {
        std::string name_;
        std::string save_file_path_;

    public:
        world(std::filesystem::path const &save_path);

        [[nodiscard]] auto get_name() const noexcept -> std::string {
            return name_;
        }

        [[nodiscard]] auto get_save_file_path() const noexcept -> std::string {
            return save_file_path_;
        }
    };

    class world_iterator {
        std::vector<std::filesystem::path> world_paths;
        std::size_t index = 0;

    public:
        world_iterator();

        auto operator++() -> world_iterator;
        auto operator++(int) -> world_iterator;
        auto operator*() const -> world;
        auto operator==(world_iterator const &another) const noexcept -> bool;
        auto operator!=(world_iterator const &another) const noexcept -> bool;

        auto begin() -> world_iterator;
        auto end() -> world_iterator;
    };
} // namespace pixel_terrain

#endif
