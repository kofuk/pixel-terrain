// SPDX-License-Identifier: MIT

/* Read whole mca files, and construct intermidiate representation of those,
   then decide pixel color and generate PNG image.. */

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

#include "graphics/color.hh"
#include "graphics/png.hh"
#include "image/blocks.hh"
#include "image/image.hh"
#include "image/worker.hh"
#include "logger/logger.hh"

namespace pixel_terrain::image {
    std::shared_ptr<worker::pixel_states>
    worker::scan_chunk(anvil::chunk *chunk, options const &options) const {
        int max_y = chunk->get_max_height();
        if (options.is_nether()) {
            if (max_y > 127) max_y = 127;
        }

        std::shared_ptr<pixel_states> states(new pixel_states);
        for (int z = 0; z < 16; ++z) {
            for (int x = 0; x < 16; ++x) {
                bool air_found = false;
                std::string prev_block;

                pixel_state &pixel_state = get_pixel_state(states, x, z);

                for (int y = max_y; y >= 0; --y) {
                    std::string block;
                    try {
                        block = chunk->get_block(x, y, z);
                    } catch (std::exception const &e) {
                        logger::L(logger::ERROR,
                                  "Warning: error occurred while obtaining "
                                  "block\n");
                        logger::L(logger::ERROR, "%s\n", e.what());

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
                        logger::L(0, "%s\tR\tG\tB\n", block.c_str());
                    } else {
                        std::uint_fast32_t color = color_itr->second;

                        if (pixel_state.fg_color == 0x00000000) {
                            pixel_state.fg_color = color;
                            pixel_state.top_height = y;
                            pixel_state.top_biome = chunk->get_biome(x, y, z);
                            if (is_biome_overridden(block)) {
                                pixel_state.add_flags(
                                    pixel_state::BIOME_OVERRIDDEN);
                            }
                            if ((color & 0xff) == 0xff) {
                                pixel_state.mid_color = color;
                                pixel_state.mid_height = y;
                                pixel_state.bg_color = color;
                                pixel_state.opaque_height = y;
                                break;
                            } else {
                                pixel_state.add_flags(
                                    pixel_state::IS_TRANSPARENT);
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

        return states;
    }

    void
    worker::handle_biomes(std::shared_ptr<pixel_states> pixel_states) const {
        /* process biome color overrides */
        for (int z = 0; z < 16; ++z) {
            for (int x = 0; x < 16; ++x) {
                pixel_state &pixel_state = get_pixel_state(pixel_states, x, z);
                if (pixel_state.get_flag(pixel_state::BIOME_OVERRIDDEN)) {
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

    void worker::handle_inclination(
        std::shared_ptr<pixel_states> pixel_states) const {
        for (int z = 0; z < 16; ++z) {
            for (int x = 1; x < 16; ++x) {
                pixel_state &left = get_pixel_state(pixel_states, x - 1, z);
                pixel_state &cur = get_pixel_state(pixel_states, x, z);
                if (left.opaque_height < cur.opaque_height) {
                    cur.bg_color = increase_brightness(cur.bg_color, 30);
                    if (x == 1) {
                        left.bg_color = increase_brightness(left.bg_color, 30);
                    }
                } else if (cur.opaque_height < left.opaque_height) {
                    cur.bg_color = increase_brightness(cur.bg_color, -30);
                    if (x == 1) {
                        left.bg_color = increase_brightness(left.bg_color, -30);
                    }
                }
            }
        }

        for (int z = 1; z < 16; ++z) {
            for (int x = 0; x < 16; ++x) {
                pixel_state &cur = get_pixel_state(pixel_states, x, z);
                pixel_state &upper = get_pixel_state(pixel_states, x, z - 1);

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

    void
    worker::process_pipeline(std::shared_ptr<pixel_states> pixel_states) const {
        handle_biomes(pixel_states);
        handle_inclination(pixel_states);
    }

    void worker::generate_image(int chunk_x, int chunk_z,
                                std::shared_ptr<pixel_states> pixel_states,
                                png &image) const {
        for (int z = 0; z < 16; ++z) {
            for (int x = 0; x < 16; ++x) {
                pixel_state &pixel_state = get_pixel_state(pixel_states, x, z);

                std::uint_fast32_t bg_color = increase_brightness(
                    pixel_state.mid_color,
                    (pixel_state.mid_height - pixel_state.top_height) * 3);
                std::uint_fast32_t color =
                    blend_color(pixel_state.fg_color, bg_color);
                bg_color = increase_brightness(
                    pixel_state.bg_color,
                    (pixel_state.opaque_height - pixel_state.top_height) * 3);
                color = blend_color(color, bg_color);
                image.set_pixel(chunk_x * 16 + x, chunk_z * 16 + z, color);
            }
        }
    }

    void worker::generate_chunk(anvil::chunk *chunk, int chunk_x, int chunk_z,
                                png &image, options const &options) const {
        std::shared_ptr<pixel_states> pixel_states = scan_chunk(chunk, options);
        process_pipeline(pixel_states);
        generate_image(chunk_x, chunk_z, pixel_states, image);
    }

    void worker::generate_region(region_container *item) const {
        anvil::region *region = item->get_region();
        logger::L(logger::DEBUG, "generating %s...\n",
                  item->get_output_path()->filename().string().c_str());

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
                    logger::L(
                        logger::DEBUG, "Warning: parse error in %s\n",
                        item->get_output_path()->filename().string().c_str());
                    logger::L(logger::DEBUG, "%s\n", e.what());
                    continue;
                }

                if (chunk == nullptr) {
                    logger::record_stat(false, item->get_options()->label());
                    continue;
                }

                if (image == nullptr) {
                    if (std::filesystem::exists(*item->get_output_path())) {
                        try {
                            image = new png(*item->get_output_path());
                            image->fit(512, 512);

                        } catch (std::exception const &) {
                            image = new png(512, 512);
                        }
                    } else {
                        image = new png(512, 512);
                    }
                }

                logger::record_stat(true, item->get_options()->label());
                generate_chunk(chunk, chunk_x, chunk_z, *image,
                               *item->get_options());

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
                            logger::L(logger::DEBUG,
                                      "Warning: parse error in %s\n",
                                      item->get_output_path()
                                          ->filename()
                                          .string()
                                          .c_str());
                            logger::L(logger::DEBUG, "%s\n", e.what());
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
            logger::L(logger::DEBUG,
                      "exiting without generating; any chunk changed in %s\n",
                      item->get_output_path()->filename().string().c_str());

            return;
        }

        image->save(*item->get_output_path());
        delete image;

        logger::L(logger::DEBUG, "generated %s\n",
                  item->get_output_path()->filename().string().c_str());
    }

} // namespace pixel_terrain::image
