/*
 * Copyright (c) 2020 Koki Fukuda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Read whole mca files, and construct intermidiate representation of those,
   then decide pixel color and generate PNG image.. */

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

#include "../utils/logger/logger.hh"
#include "blocks.hh"
#include "color.hh"
#include "generator.hh"
#include "png.hh"
#include "worker.hh"

namespace pixel_terrain::image {
    struct PixelState {
        std::uint_fast32_t flags;
        std::uint_fast8_t top_height;
        std::uint_fast8_t mid_height;
        std::uint_fast8_t opaque_height;
        std::uint_fast32_t fg_color;
        std::uint_fast32_t mid_color;
        std::uint_fast32_t bg_color;
        std::int32_t top_biome;

        PixelState()
            : flags(0), top_height(0), mid_height(0), opaque_height(0),
              fg_color(0x00000000), mid_color(0x00000000), bg_color(0x00000000),
              top_biome(0) {}
        void add_flags(std::int_fast32_t flags) { this->flags |= flags; }
        bool get_flag(std::int_fast32_t field) { return this->flags & field; }
    };

    namespace {
        constexpr std::int32_t PS_IS_TRANSPARENT = 1;
        constexpr std::int32_t PS_BIOME_OVERRIDDEN = 1 << 1;

        inline PixelState &get_pixel_state(
            std::shared_ptr<std::array<PixelState, 512 * 512>> pixel_state,
            int x, int y) {
            return (*pixel_state)[y * 512 + x];
        }

        std::shared_ptr<std::array<PixelState, 512 * 512>>
        scan_chunk(anvil::chunk *chunk) {
            int max_y = chunk->get_max_height();
            if (option_nether) {
                if (max_y > 127) max_y = 127;
            }

            std::shared_ptr<std::array<PixelState, 512 * 512>> pixel_states(
                new std::array<PixelState, 512 * 512>);
            for (int z = 0; z < 16; ++z) {
                for (int x = 0; x < 16; ++x) {
                    bool air_found = false;
                    std::string prev_block;

                    PixelState &pixel_state =
                        get_pixel_state(pixel_states, x, z);

                    for (int y = max_y; y >= 0; --y) {
                        std::string block;
                        try {
                            block = chunk->get_block(x, y, z);
                        } catch (std::exception const &e) {
                            logger::e("Warning: error occurred while obtaining "
                                      "block");
                            logger::e(e.what());

                            continue;
                        }

                        if (block == "minecraft:air" ||
                            block == "minecraft:cave_air" ||
                            block == "minecraft:void_air") {
                            air_found = true;
                            prev_block = block;
                            continue;
                        }

                        if (option_nether && !air_found) {
                            continue;
                        }

                        if (block == prev_block) {
                            continue;
                        }

                        prev_block = block;

                        auto color_itr = colors.find(block);
                        if (color_itr == end(colors)) {
                            logger::i(R"(colors[")" + block + R"("] = ???)");
                        } else {
                            std::uint_fast32_t color = color_itr->second;

                            if (pixel_state.fg_color == 0x00000000) {
                                pixel_state.fg_color = color;
                                pixel_state.top_height = y;
                                pixel_state.top_biome =
                                    chunk->get_biome(x, y, z);
                                if (is_biome_overridden(block)) {
                                    pixel_state.add_flags(PS_BIOME_OVERRIDDEN);
                                }
                                if ((color & 0xff) == 0xff) {
                                    pixel_state.mid_color = color;
                                    pixel_state.mid_height = y;
                                    pixel_state.bg_color = color;
                                    pixel_state.opaque_height = y;
                                    break;
                                } else {
                                    pixel_state.add_flags(PS_IS_TRANSPARENT);
                                }
                            } else if (pixel_state.mid_color == 0x00000000) {
                                pixel_state.mid_color = color;
                                pixel_state.mid_height = y;
                                if ((color & 0xff) == 0xff) {
                                    pixel_state.bg_color = color;
                                    pixel_state.opaque_height = y;
                                    break;
                                }
                            } else {
                                pixel_state.bg_color =
                                    blend_color(pixel_state.bg_color, color);
                                if ((pixel_state.bg_color & 0xff) == 0xff) {
                                    pixel_state.opaque_height = y;
                                    break;
                                }
                            }
                        }
                    }
                    pixel_state.bg_color |= 0xff;
                    if (pixel_state.top_height == pixel_state.opaque_height) {
                        pixel_state.fg_color = 0x00000000;
                        pixel_state.mid_color = 0x00000000;
                    } else if (pixel_state.mid_height ==
                               pixel_state.opaque_height) {
                        pixel_state.mid_color = 0x00000000;
                    }
                }
            }

            return pixel_states;
        }

        void handle_biomes(
            std::shared_ptr<std::array<PixelState, 512 * 512>> pixel_states) {
            /* process biome color overrides */
            for (int z = 0; z < 16; ++z) {
                for (int x = 0; x < 16; ++x) {
                    PixelState &pixel_state =
                        get_pixel_state(pixel_states, x, z);
                    if (pixel_state.get_flag(PS_BIOME_OVERRIDDEN)) {
                        std::uint_fast32_t *color;
                        if (pixel_state.fg_color != 0x00000000) {
                            color = &(pixel_state.fg_color);
                        } else {
                            color = &(pixel_state.bg_color);
                        }
                        std::int32_t biome = pixel_state.top_biome;
                        if (biome == 6 || biome == 134) {
                            *color = blend_color(*color, 0x665956ff, 0.5);
                        } else if (biome == 21 || biome == 149 || biome == 23 ||
                                   biome == 151) {
                            *color = blend_color(*color, 0x83bd7eff, 0.5);
                        } else if (biome == 35 || biome == 163) {
                            *color = blend_color(*color, 0xa8ab33ff, 0.5);
                        }
                    }
                }
            }
        }

        void handle_inclination(
            std::shared_ptr<std::array<PixelState, 512 * 512>> pixel_states) {
            for (int z = 0; z < 16; ++z) {
                for (int x = 1; x < 16; ++x) {
                    PixelState &left = get_pixel_state(pixel_states, x - 1, z);
                    PixelState &cur = get_pixel_state(pixel_states, x, z);
                    if (left.opaque_height < cur.opaque_height) {
                        cur.bg_color = increase_brightness(cur.bg_color, 30);
                        if (x == 1) {
                            left.bg_color =
                                increase_brightness(left.bg_color, 30);
                        }
                    } else if (cur.opaque_height < left.opaque_height) {
                        cur.bg_color = increase_brightness(cur.bg_color, -30);
                        if (x == 1) {
                            left.bg_color =
                                increase_brightness(left.bg_color, -30);
                        }
                    }
                }
            }

            for (int z = 1; z < 16; ++z) {
                for (int x = 0; x < 16; ++x) {
                    PixelState &cur = get_pixel_state(pixel_states, x, z);
                    PixelState &upper = get_pixel_state(pixel_states, x, z - 1);

                    if (upper.opaque_height < cur.opaque_height) {
                        cur.bg_color = increase_brightness(cur.bg_color, 10);
                        if (z == 1) {
                            upper.bg_color =
                                increase_brightness(upper.bg_color, 10);
                        }
                    } else if (cur.opaque_height < upper.opaque_height) {
                        cur.bg_color = increase_brightness(cur.bg_color, -10);
                        if (z == 1) {
                            upper.bg_color =
                                increase_brightness(upper.bg_color, -10);
                        }
                    }
                }
            }
        }

        void process_pipeline(
            std::shared_ptr<std::array<PixelState, 512 * 512>> pixel_states) {
            handle_biomes(pixel_states);
            handle_inclination(pixel_states);
        }

        void generate_image(
            int chunk_x, int chunk_z,
            std::shared_ptr<std::array<PixelState, 512 * 512>> pixel_states,
            png &image) {
            for (int z = 0; z < 16; ++z) {
                for (int x = 0; x < 16; ++x) {
                    PixelState &pixel_state =
                        get_pixel_state(pixel_states, x, z);

                    std::uint_fast32_t bg_color = increase_brightness(
                        pixel_state.mid_color,
                        (pixel_state.mid_height - pixel_state.top_height) * 3);
                    std::uint_fast32_t color =
                        blend_color(pixel_state.fg_color, bg_color);
                    bg_color = increase_brightness(
                        pixel_state.bg_color,
                        (pixel_state.opaque_height - pixel_state.top_height) *
                            3);
                    color = blend_color(color, bg_color);
                    image.set_pixel(chunk_x * 16 + x, chunk_z * 16 + z, color);
                }
            }
        }

        void generate_chunk(anvil::chunk *chunk, int chunk_x, int chunk_z,
                            png &image) {
            std::shared_ptr<std::array<PixelState, 512 * 512>> pixel_states =
                scan_chunk(chunk);
            process_pipeline(pixel_states);
            generate_image(chunk_x, chunk_z, pixel_states, image);
        }
    } // namespace

    void generate_256(std::shared_ptr<queued_item> item) {
        anvil::region *region = item->region->region;
        int region_x = item->region->rx;
        int region_z = item->region->rz;

        if (option_verbose) {
            logger::d("generating " + item->debug_string() + " ...");
        }

        std::filesystem::path path = option_out_dir;
        {
            path_string name;
            name.append(std::filesystem::path(std::to_string(region_x)));
            name.append(PATH_STR_LITERAL(","));
            name.append(std::filesystem::path(std::to_string(region_z)));
            name.append(PATH_STR_LITERAL(".png"));
            path /= name;
        }

        png *image = nullptr;

        /* minumum range of chunk update is radius of 3, so we can capture
           all updated chunk with step of 6. but, we set this 4 since
           4 can divide 16, our image (chunk) width. */
        for (int chunk_z = 0; chunk_z < 32; chunk_z += 4) {
            int prev_chunk_x = -1;
            for (int chunk_x = 0; chunk_x < 32; ++chunk_x) {
                anvil::chunk *chunk;

                try {
                    /* Avoid nonexisting chunk to be recorded as reused chunk.
                     */
                    if (region->exists_chunk_data(chunk_x, chunk_z)) {
                        continue;
                    }

                    chunk = region->get_chunk_if_dirty(chunk_x, chunk_z);
                } catch (std::exception const &e) {
                    logger::e("Warning: parse error in " +
                              item->debug_string());
                    logger::e(e.what());
                    continue;
                }

                if (chunk == nullptr) {
                    logger::record_stat(false);
                    continue;
                }

                if (image == nullptr) {
                    if (std::filesystem::exists(path)) {
                        try {
                            image = new png(path);

                        } catch (std::exception const &) {
                            image = new png(512, 512);
                        }
                    } else {
                        image = new png(512, 512);
                    }
                }

                logger::record_stat(true);
                generate_chunk(chunk, chunk_x, chunk_z, *image);

                delete chunk;

                int start_x =
                    chunk_x - 3 > prev_chunk_x ? chunk_x - 3 : prev_chunk_x + 1;
                int end_x = chunk_x + 4 > 32 ? 32 : chunk_x + 4;

                for (int t_chunk_x = start_x; t_chunk_x < end_x; ++t_chunk_x) {
                    for (int t_chunk_z = chunk_z + 1; t_chunk_z < chunk_z + 4;
                         ++t_chunk_z) {
                        try {
                            if (region->exists_chunk_data(t_chunk_x,
                                                          t_chunk_z)) {
                                continue;
                            }

                            chunk = region->get_chunk_if_dirty(t_chunk_x,
                                                               t_chunk_z);
                        } catch (std::exception const &e) {
                            logger::e("Warning: parse error in " +
                                      item->debug_string());
                            logger::e(e.what());
                            continue;
                        }

                        if (chunk == nullptr) {
                            continue;
                        }

                        logger::record_stat(true);
                        generate_chunk(chunk, t_chunk_x, t_chunk_z, *image);

                        delete chunk;
                    }
                }
                prev_chunk_x = chunk_x + 3;
            }
        }

        if (image == nullptr) {
            if (option_verbose) {
                logger::d("exiting without generating; any chunk changed in " +
                          item->debug_string());
            }

            return;
        }

        image->save(path);
        delete image;

        if (option_verbose) {
            logger::d("generated " + item->debug_string());
        }
    }

} // namespace pixel_terrain::image
