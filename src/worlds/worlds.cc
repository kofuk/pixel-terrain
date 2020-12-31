// SPDX-License-Identifier: MIT

#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>

#include "nbt/pull_parser/nbt_pull_parser.hh"
#include "nbt/utils.hh"
#include "utils/path_hack.hh"
#include "worlds.hh"

namespace pixel_terrain {
    namespace {
        auto get_save_dir() -> std::filesystem::path {
#ifdef OS_LINUX
            std::filesystem::path base = std::getenv("HOME");
#elif defined(OS_WIN)
            std::filesystem::path base = std::getenv("APPDATA");
#endif
            base /= PATH_STR_LITERAL(".minecraft");
            base /= PATH_STR_LITERAL("saves");

            return base;
        }

        auto read_world_name(std::filesystem::path const &save_path)
            -> std::string {
            std::filesystem::path level_dat_path =
                save_path / PATH_STR_LITERAL("level.dat");
            auto [data, len] = nbt::utils::gzip_file_decompress(level_dat_path);
            if (len == 0) {
                throw std::runtime_error("Cannot load level.dat");
            }
            nbt::nbt_pull_parser parser(data, len);

            int nest_level = 0;
            std::array<std::string, 3> nbt_path = {"", "Data", "LevelName"};
            int should_satisfy_level = 0;
            for (;;) {
                nbt::parser_event ev = parser.next();
                if (ev == nbt::parser_event::TAG_START) {
                    if (nest_level < 3) {
                        if (parser.get_tag_name() == nbt_path[nest_level]) {
                            ++should_satisfy_level;
                            if (should_satisfy_level == 3) {
                                parser.next();
                                break;
                            }
                        }
                    }

                    ++nest_level;
                } else if (ev == nbt::parser_event::TAG_END) {
                    --nest_level;
                    if (nest_level < should_satisfy_level) {
                        throw std::runtime_error("Invalid level.dat");
                    }
                }
            }

            if (should_satisfy_level == 3 &&
                parser.get_event_type() == nbt::parser_event::DATA &&
                parser.get_tag_type() == nbt::TAG_STRING) {
                std::string name = parser.get_string();
                /* XXX: Avoid memory leak. (I'll rewrite NBT parser soon; this
                        is temporary implementation) */
                parser.next();
                return name;
            }

            throw std::runtime_error("Invalid level.dat");
        }
    } // namespace

    world::world(std::filesystem::path const &save_path) {
        name_ = read_world_name(save_path);
        save_file_path_ = save_path.string();
    }

    world_iterator::world_iterator() {
        std::unique_ptr<std::filesystem::directory_iterator> dirents;
        try {
            dirents = std::make_unique<std::filesystem::directory_iterator>(
                get_save_dir());
        } catch (std::filesystem::filesystem_error const &e) {
            /* There's no saved worlds. */
            return;
        }

        for (std::filesystem::directory_entry const &entry : *dirents) {
            if (entry.is_directory() &&
                std::filesystem::exists(entry.path() /
                                        PATH_STR_LITERAL("level.dat"))) {
                world_paths.push_back(entry);
            }
        }

        std::sort(std::begin(world_paths), std::end(world_paths));
    }

    auto world_iterator::operator++() -> world_iterator {
        ++index;
        return *this;
    }

    auto world_iterator::operator++(int) -> world_iterator {
        world_iterator tmp = *this;
        ++index;
        return tmp;
    }

    auto world_iterator::operator*() const -> world {
        return world(world_paths[index]);
    }

    auto world_iterator::operator==(world_iterator const &another) const
        noexcept(true) -> bool {
        return index == another.index;
    }

    auto world_iterator::operator!=(world_iterator const &another) const
        noexcept(true) -> bool {
        return index != another.index;
    }

    auto world_iterator::begin() -> world_iterator {
        world_iterator b = *this;
        b.index = 0;
        return b;
    }

    auto world_iterator::end() -> world_iterator {
        world_iterator e = *this;
        e.index = e.world_paths.size();
        return e;
    }
} // namespace pixel_terrain
