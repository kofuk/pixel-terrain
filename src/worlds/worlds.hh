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
        world(std::filesystem::path const &);

        std::string get_name() const noexcept {
            return name_;
        }

        std::string get_save_file_path() const noexcept {
            return save_file_path_;
        }
    };

    class world_iterator {
        std::vector<std::filesystem::path> world_paths;
        std::size_t index = 0;

    public:
        world_iterator();

        world_iterator operator++();
        world_iterator operator++(int);
        world operator*() const;
        bool operator==(world_iterator const &) const noexcept;
        bool operator!=(world_iterator const &) const noexcept;

        world_iterator begin();
        world_iterator end();
    };
} // namespace pixel_terrain

#endif
