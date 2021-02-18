// SPDX-License-Identifier: MIT

#include "nbt/tag.hh"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#if USE_V3_NBT_PARSER
#include "nbt/nbt-path.hh"
#include "nbt/nbt.hh"
#else
#include "nbt/pull_parser/nbt_pull_parser.hh"
#endif
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
    } // namespace

    auto world::parse_level_dat(std::filesystem::path const &save_path)
        -> bool {
        std::filesystem::path level_dat_path =
            save_path / PATH_STR_LITERAL("level.dat");
        std::vector<std::uint8_t> *data =
            nbt::utils::gzip_file_decompress(level_dat_path);
        if (data == nullptr) {
            return false;
        }
#if USE_V3_NBT_PARSER
        auto *leveldat = nbt::nbt::from_iterator(data->cbegin(), data->cend());
        auto *world_name_node = leveldat->query<nbt::tag_string_payload>(
            nbt::nbt_path::compile("//Data/LevelName"));
        if (world_name_node == nullptr) {
            return false;
        }
        name_ = **world_name_node;
        delete world_name_node;

        auto *dimensions_node = leveldat->query<nbt::tag_compound_payload>(
            nbt::nbt_path::compile("//Data/WorldGenSettings/dimensions"));
        if (dimensions_node != nullptr) {
            for (auto *tag : **dimensions_node) {
                dimensions_.push_back(tag->name());
            }
        }
        delete dimensions_node;

        auto *game_version_node = leveldat->query<nbt::tag_string_payload>(
            nbt::nbt_path::compile("//Data/Version/Name"));
        if (game_version_node != nullptr) {
            game_version_ = **game_version_node;
        }
        delete game_version_node;

        auto *hardcore_node = leveldat->query<nbt::tag_byte_payload>(
            nbt::nbt_path::compile("//Data/hardcore"));
        if (hardcore_node != nullptr) {
            is_hardcore_ = **hardcore_node != 0;
        }
        delete hardcore_node;

        auto *datapacks_node = leveldat->query<nbt::tag_compound_payload>(
            nbt::nbt_path::compile("//Data/DataPacks"));
        if (datapacks_node != nullptr) {

            auto *enabled_datapacks_node =
                datapacks_node->query<nbt::tag_list_payload>(
                    nbt::nbt_path::compile("/Enabled"));
            if (enabled_datapacks_node != nullptr &&
                enabled_datapacks_node->payload_type() ==
                    nbt::tag_type::TAG_STRING) {
                for (auto *p : **enabled_datapacks_node) {
                    enabled_datapacks_.push_back(
                        **static_cast<nbt::tag_string_payload *>(p));
                }
            }
            delete enabled_datapacks_node;

            auto *disabled_datapacks_node =
                datapacks_node->query<nbt::tag_list_payload>(
                    nbt::nbt_path::compile("/Disabled"));
            if (disabled_datapacks_node != nullptr &&
                disabled_datapacks_node->payload_type() ==
                    nbt::tag_type::TAG_STRING) {
                for (auto *p : **disabled_datapacks_node) {
                    disabled_datapacks_.push_back(
                        **static_cast<nbt::tag_string_payload *>(p));
                }
            }
            delete disabled_datapacks_node;

            delete datapacks_node;
        }

        delete leveldat;
        delete data;

        return true;
#else  // not USE_V3_NBT_PARSER
        nbt::nbt_pull_parser parser(data->data(), data->size());

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
                    return false;
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
            name_ = name;
        }

        return false;
#endif // USE_V3_NBT_PARSER
    }

    world::world(std::filesystem::path const &save_path) {
        parse_level_dat(save_path);
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
