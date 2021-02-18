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
        std::vector<std::string> dimensions_;
        std::string game_version_;
        bool is_hardcore_;
        std::vector<std::string> enabled_datapacks_;
        std::vector<std::string> disabled_datapacks_;

        auto parse_level_dat(std::filesystem::path const &) -> bool;

    public:
        world(std::filesystem::path const &save_path);

        [[nodiscard]] auto get_name() const noexcept -> std::string {
            return name_;
        }

        [[nodiscard]] auto get_save_file_path() const noexcept -> std::string {
            return save_file_path_;
        }

        [[nodiscard]] auto get_dimensions() const noexcept
            -> std::vector<std::string> {
            return dimensions_;
        }

        [[nodiscard]] auto get_game_version() const noexcept
            -> std::string const & {
            return game_version_;
        }

        [[nodiscard]] auto is_hardcore() const noexcept -> bool {
            return is_hardcore_;
        }

        [[nodiscard]] auto get_enabled_datapacks() const noexcept
            -> std::vector<std::string> const & {
            return enabled_datapacks_;
        }

        [[nodiscard]] auto get_disabled_datapacks() const noexcept
            -> std::vector<std::string> const & {
            return disabled_datapacks_;
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
