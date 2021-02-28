// SPDX-License-Identifier: MIT

/* Read whole mca files, and construct intermidiate representation of those,
   then decide pixel color and generate PNG image.. */

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

#include "graphics/color.hh"
#include "graphics/constants.hh"
#include "graphics/png.hh"
#include "image/blocks.hh"
#include "image/image.hh"
#include "image/worker.hh"
#include "logger/logger.hh"
#include "nbt/constants.hh"

namespace pixel_terrain::image {
    auto worker::scan_chunk(anvil::chunk *chunk, options const &options) const
        -> worker::pixel_states * {
        using namespace graphics;

        int max_y = chunk->get_max_height();
        if (options.is_nether()) {
            if (max_y > nbt::biomes::CHUNK_MAX_Y_NETHER) {
                max_y = nbt::biomes::CHUNK_MAX_Y_NETHER;
            }
        }

        auto *states = new pixel_states;
        for (int z = 0; z < nbt::biomes::CHUNK_WIDTH; ++z) {
            for (int x = 0; x < nbt::biomes::CHUNK_WIDTH; ++x) {
                bool air_found = false;
                std::string prev_block;

                pixel_state &pixel_state = get_pixel_state(states, x, z);

                for (int y = max_y; y >= 0; --y) {
                    std::string block;
                    try {
                        block = chunk->get_block(x, y, z);
                    } catch (std::exception const &e) {
                        ELOG("Error occurred while obtaining block\n");
                        ELOG("%s\n", e.what());

                        continue;
                    }

                    if (block == "minecraft:air" ||
                        block == "minecraft:cave_air" ||
                        block == "minecraft:void_air") {
                        air_found = true;
                        prev_block = block;
                        continue;
                    }

                    if (options.is_nether() && !air_found) {
                        continue;
                    }

                    if (block == prev_block) {
                        continue;
                    }

                    prev_block = block;

                    auto color_itr = colors.find(block);
                    if (color_itr == end(colors)) {
                        std::unique_lock<std::mutex> lock(
                            unknown_blocks_mutex_);
                        unknown_blocks_.insert(block);
                    } else {
                        std::uint_fast32_t color = color_itr->second;

                        if (pixel_state.fg_color() == 0x00000000) {
                            pixel_state.set_fg_color(color);
                            pixel_state.set_top_height(y);
                            pixel_state.set_top_biome(
                                chunk->get_biome(x, y, z));
                            if (is_biome_overridden(block)) {
                                pixel_state.add_flags(
                                    pixel_state::BIOME_OVERRIDDEN);
                            }
                            if (graphics::alpha(color) == color::CHAN_FULL) {
                                pixel_state.set_mid_color(color);
                                pixel_state.set_mid_height(y);
                                pixel_state.set_bg_color(color);
                                pixel_state.set_opaque_height(y);
                                break;
                            }

                            pixel_state.add_flags(pixel_state::IS_TRANSPARENT);
                        } else if (pixel_state.mid_color() == color::CHAN_MIN) {
                            pixel_state.set_mid_color(color);
                            pixel_state.set_mid_height(y);
                            if ((color & color::CHAN_MASK) ==
                                color::CHAN_FULL) {
                                pixel_state.set_bg_color(color);
                                pixel_state.set_opaque_height(y);
                                break;
                            }
                        } else {
                            pixel_state.set_bg_color(
                                blend_color(pixel_state.bg_color(), color));
                            if ((pixel_state.bg_color() & color::CHAN_MASK) ==
                                color::CHAN_FULL) {
                                pixel_state.set_opaque_height(y);
                                break;
                            }
                        }
                    }
                }
                pixel_state.set_bg_color(pixel_state.bg_color() |
                                         color::CHAN_FULL);
                if (pixel_state.top_height() == pixel_state.opaque_height()) {
                    pixel_state.set_fg_color(color::CHAN_MIN);
                    pixel_state.set_mid_color(color::CHAN_MIN);
                } else if (pixel_state.mid_height() ==
                           pixel_state.opaque_height()) {
                    pixel_state.set_mid_color(0x00000000);
                }
            }
        }

        return states;
    }

    void worker::handle_biomes(pixel_states *pixel_states) {
        using namespace graphics;

        /* process biome color overrides */
        for (int z = 0; z < nbt::biomes::CHUNK_WIDTH; ++z) {
            for (int x = 0; x < nbt::biomes::CHUNK_WIDTH; ++x) {
                pixel_state &pixel_state = get_pixel_state(pixel_states, x, z);
                if (pixel_state.get_flag(pixel_state::BIOME_OVERRIDDEN)) {
                    std::uint32_t src_color;
                    if (pixel_state.fg_color() != color::CHAN_MIN) {
                        src_color = pixel_state.fg_color();
                    } else {
                        src_color = pixel_state.bg_color();
                    }
                    std::int32_t biome = pixel_state.top_biome();
                    constexpr double mix_half = 0.5;
                    if (biome == nbt::biomes::SWAMP ||
                        biome == nbt::biomes::SWAMP_HILLS) {
                        src_color = blend_color(
                            src_color, nbt::biomes::overrides::SWAMP, mix_half);
                    } else if (biome == nbt::biomes::JUNGLE ||
                               biome == nbt::biomes::MODIFIED_JUNGLE ||
                               biome == nbt::biomes::JUNGLE_EDGE ||
                               biome == nbt::biomes::MODIFIED_JUNGLE_EDGE) {
                        src_color = blend_color(src_color,
                                                nbt::biomes::overrides::JUNGLE,
                                                mix_half);
                    } else if (biome == nbt::biomes::SAVANNA ||
                               biome == nbt::biomes::SHATTERED_SAVANNA) {
                        src_color = blend_color(src_color,
                                                nbt::biomes::overrides::SAVANNA,
                                                mix_half);
                    }
                    if (pixel_state.fg_color() != color::CHAN_MIN) {
                        pixel_state.set_fg_color(src_color);
                    } else {
                        pixel_state.set_bg_color(src_color);
                    }
                }
            }
        }
    }

    void worker::handle_inclination(pixel_states *pixel_states) {
        constexpr int x_tone_change_ratio = 30;
        constexpr int z_tone_change_ratio = 10;

        for (int z = 0; z < nbt::biomes::CHUNK_WIDTH; ++z) {
            for (int x = 1; x < nbt::biomes::CHUNK_WIDTH; ++x) {
                pixel_state &left = get_pixel_state(pixel_states, x - 1, z);
                pixel_state &cur = get_pixel_state(pixel_states, x, z);
                if (left.opaque_height() < cur.opaque_height()) {
                    cur.set_bg_color(graphics::increase_brightness(
                        cur.bg_color(), x_tone_change_ratio));
                    if (x == 1) {
                        left.set_bg_color(graphics::increase_brightness(
                            left.bg_color(), x_tone_change_ratio));
                    }
                } else if (cur.opaque_height() < left.opaque_height()) {
                    cur.set_bg_color(graphics::increase_brightness(
                        cur.bg_color(), -x_tone_change_ratio));
                    if (x == 1) {
                        left.set_bg_color(graphics::increase_brightness(
                            left.bg_color(), -x_tone_change_ratio));
                    }
                }
            }
        }

        for (int z = 1; z < nbt::biomes::CHUNK_WIDTH; ++z) {
            for (int x = 0; x < nbt::biomes::CHUNK_WIDTH; ++x) {
                pixel_state &cur = get_pixel_state(pixel_states, x, z);
                pixel_state &upper = get_pixel_state(pixel_states, x, z - 1);

                if (upper.opaque_height() < cur.opaque_height()) {
                    cur.set_bg_color(graphics::increase_brightness(
                        cur.bg_color(), z_tone_change_ratio));
                    if (z == 1) {
                        upper.set_bg_color(graphics::increase_brightness(
                            upper.bg_color(), z_tone_change_ratio));
                    }
                } else if (cur.opaque_height() < upper.opaque_height()) {
                    cur.set_bg_color(graphics::increase_brightness(
                        cur.bg_color(), -z_tone_change_ratio));
                    if (z == 1) {
                        upper.set_bg_color(graphics::increase_brightness(
                            upper.bg_color(), -z_tone_change_ratio));
                    }
                }
            }
        }
    }

    void worker::process_pipeline(pixel_states *pixel_states) {
        handle_biomes(pixel_states);
        handle_inclination(pixel_states);
    }

    void worker::generate_image(int chunk_x, int chunk_z,
                                pixel_states *pixel_states,
                                graphics::png &image) {
        constexpr int height_tone_ratio = 3;

        for (int z = 0; z < nbt::biomes::CHUNK_WIDTH; ++z) {
            for (int x = 0; x < nbt::biomes::CHUNK_WIDTH; ++x) {
                pixel_state &pixel_state = get_pixel_state(pixel_states, x, z);

                std::uint_fast32_t bg_color = graphics::increase_brightness(
                    pixel_state.mid_color(),
                    static_cast<int>(pixel_state.mid_height() -
                                     pixel_state.top_height()) *
                        height_tone_ratio);
                std::uint_fast32_t color =
                    graphics::blend_color(pixel_state.fg_color(), bg_color);
                bg_color = graphics::increase_brightness(
                    pixel_state.bg_color(),
                    static_cast<int>(pixel_state.opaque_height() -
                                     pixel_state.top_height()) *
                        height_tone_ratio);
                color = graphics::blend_color(color, bg_color);
                image.set_pixel(chunk_x * nbt::biomes::CHUNK_WIDTH + x,
                                chunk_z * nbt::biomes::CHUNK_WIDTH + z, color);
            }
        }
    }

    void worker::generate_chunk(anvil::chunk *chunk, int chunk_x, int chunk_z,
                                graphics::png &image,
                                options const &options) const {
        pixel_states *pixel_states = scan_chunk(chunk, options);
        process_pipeline(pixel_states);
        generate_image(chunk_x, chunk_z, pixel_states, image);
        delete pixel_states;
    }

    worker::~worker() {
        if (!unknown_blocks_.empty()) {
            ILOG("Unknown blocks:\n");
            for (std::string const &block : unknown_blocks_) {
                ILOG(" %s\n", block.c_str());
            }
        }
    }

    void worker::generate_region(region_container *item) const {
        anvil::region *region = item->get_region();
        DLOG("Generating %s...\n",
             item->get_output_path()->filename().string().c_str());

        graphics::png *image = nullptr;

        /* minumum range of chunk update is radius of 3, so we can capture
           all updated chunk with step of 6. but, we set this 4 since
           4 can divide 16, our image (chunk) width. */
        constexpr int scan_chunk_step = 4;
        for (int chunk_z = 0; chunk_z < nbt::biomes::CHUNK_PER_REGION_WIDTH;
             chunk_z += scan_chunk_step) {
            int prev_chunk_x = -1;
            for (int chunk_x = 0; chunk_x < nbt::biomes::CHUNK_PER_REGION_WIDTH;
                 ++chunk_x) {
                anvil::chunk *chunk;

                try {
                    /* Avoid nonexisting chunk to be recorded as reused chunk.
                     */
                    if (region->is_chunk_missing(chunk_x, chunk_z)) {
                        continue;
                    }

                    chunk = region->get_chunk_if_dirty(chunk_x, chunk_z);
                } catch (std::exception const &e) {
                    DLOG("Warning: parse error in %s\n",
                         item->get_output_path()->filename().string().c_str());
                    DLOG("%s\n", e.what());
                    continue;
                }

                if (chunk == nullptr) {
                    logger::record_stat(false, item->get_options()->label());
                    continue;
                }

                if (image == nullptr) {
                    if (std::filesystem::exists(*item->get_output_path())) {
                        try {
                            image = new graphics::png(*item->get_output_path());
                            image->fit(nbt::biomes::BLOCK_PER_REGION_WIDTH,
                                       nbt::biomes::BLOCK_PER_REGION_WIDTH);

                        } catch (std::exception const &) {
                            image = new graphics::png(
                                nbt::biomes::BLOCK_PER_REGION_WIDTH,
                                nbt::biomes::BLOCK_PER_REGION_WIDTH);
                        }
                    } else {
                        image = new graphics::png(
                            nbt::biomes::BLOCK_PER_REGION_WIDTH,
                            nbt::biomes::BLOCK_PER_REGION_WIDTH);
                    }
                }

                logger::record_stat(true, item->get_options()->label());
                generate_chunk(chunk, chunk_x, chunk_z, *image,
                               *item->get_options());

                delete chunk;

                int start_x = chunk_x - scan_chunk_step + 1 > prev_chunk_x
                                  ? chunk_x - scan_chunk_step + 1
                                  : prev_chunk_x + 1;
                int end_x = chunk_x + scan_chunk_step >
                                    nbt::biomes::CHUNK_PER_REGION_WIDTH
                                ? nbt::biomes::CHUNK_PER_REGION_WIDTH
                                : chunk_x + scan_chunk_step;

                for (int t_chunk_x = start_x; t_chunk_x < end_x; ++t_chunk_x) {
                    for (int t_chunk_z = chunk_z + 1; t_chunk_z < chunk_z + 4;
                         ++t_chunk_z) {
                        try {
                            if (region->is_chunk_missing(t_chunk_x,
                                                         t_chunk_z)) {
                                continue;
                            }

                            chunk = region->get_chunk_if_dirty(t_chunk_x,
                                                               t_chunk_z);
                        } catch (std::exception const &e) {
                            DLOG("Parse error in %s\n", item->get_output_path()
                                                            ->filename()
                                                            .string()
                                                            .c_str());
                            DLOG("%s\n", e.what());
                            continue;
                        }

                        if (chunk == nullptr) {
                            continue;
                        }

                        logger::record_stat(true, item->get_options()->label());
                        generate_chunk(chunk, t_chunk_x, t_chunk_z, *image,
                                       *item->get_options());

                        delete chunk;
                    }
                }
                prev_chunk_x = chunk_x + 3;
            }
        }

        if (image == nullptr) {
            DLOG("Exiting without generating; any chunk changed in %s\n",
                 item->get_output_path()->filename().string().c_str());

            return;
        }

        image->save(*item->get_output_path());
        delete image;

        DLOG("Generated %s\n",
             item->get_output_path()->filename().string().c_str());
    }

} // namespace pixel_terrain::image
