/* Read whole mca files, and construct intermidiate representation of those,
   then decide pixel color and generate PNG image.. */

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

#include "../../logger/logger.hh"
#include "PNG.hh"
#include "blocks.hh"
#include "color.hh"
#include "generator.hh"

namespace generator {
    struct PixelState {
        uint_fast32_t flags;
        uint_fast8_t top_height;
        uint_fast8_t mid_height;
        uint_fast8_t opaque_height;
        uint_fast32_t fg_color;
        uint_fast32_t mid_color;
        uint_fast32_t bg_color;
        int32_t top_biome;

        PixelState ()
            : flags (0), top_height (0), mid_height (0), opaque_height (0),
              fg_color (0x00000000), mid_color (0x00000000),
              bg_color (0x00000000), top_biome (0) {}
        void add_flags (int_fast32_t flags) { this->flags |= flags; }
        bool get_flag (int_fast32_t field) { return this->flags & field; }
    };

    namespace {
        constexpr int32_t PS_IS_TRANSPARENT = 1;
        constexpr int32_t PS_BIOME_OVERRIDDEN = 1 << 1;

        inline PixelState &
        get_pixel_state (shared_ptr<array<PixelState, 256 * 256>> pixel_state,
                         int x, int y) {
            return (*pixel_state)[y * 256 + x];
        }

        shared_ptr<array<PixelState, 256 * 256>>
        scan_chunk (anvil::Chunk *chunk) {
            int max_y = chunk->get_max_height ();
            if (option_nether) {
                if (max_y > 127) max_y = 127;
            }

            shared_ptr<array<PixelState, 256 * 256>> pixel_states (
                new array<PixelState, 256 * 256>);
            for (int z = 0; z < 16; ++z) {
                for (int x = 0; x < 16; ++x) {
                    bool air_found = false;
                    string prev_block;

                    PixelState &pixel_state =
                        get_pixel_state (pixel_states, x, z);

                    for (int y = max_y; y >= 0; --y) {
                        string block;
                        try {
                            block = chunk->get_block (x, y, z);
                        } catch (exception const &e) {
                            logger::e (
                                "Warning: error occurred while obtaining block"s);
                            logger::e (e.what ());

                            continue;
                        }

                        if (block == "air"sv || block == "cave_air"sv ||
                            block == "void_air"sv) {
                            air_found = true;
                            prev_block = block;
                            continue;
                        }

                        if (block == prev_block) {
                            continue;
                        }

                        prev_block = block;

                        auto color_itr = colors.find (block);
                        if (color_itr == end (colors)) {
                            logger::i (R"(colors[")"s + block + R"("] = ???)"s);
                        } else {
                            uint_fast32_t color = color_itr->second;

                            if (pixel_state.fg_color == 0x00000000) {
                                pixel_state.fg_color = color;
                                pixel_state.top_height = y;
                                pixel_state.top_biome =
                                    chunk->get_biome (x, y, z);
                                if (is_biome_overridden (block)) {
                                    pixel_state.add_flags (PS_BIOME_OVERRIDDEN);
                                }
                                if ((color & 0xff) == 0xff) {
                                    pixel_state.mid_color = color;
                                    pixel_state.mid_height = y;
                                    pixel_state.bg_color = color;
                                    pixel_state.opaque_height = y;
                                    break;
                                } else {
                                    pixel_state.add_flags (PS_IS_TRANSPARENT);
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
                                    blend_color (pixel_state.bg_color, color);
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

        void
        handle_biomes (shared_ptr<array<PixelState, 256 * 256>> pixel_states) {
            /* process biome color overrides */
            for (int z = 0; z < 16; ++z) {
                for (int x = 0; x < 16; ++x) {
                    PixelState &pixel_state =
                        get_pixel_state (pixel_states, x, z);
                    if (pixel_state.get_flag (PS_BIOME_OVERRIDDEN)) {
                        uint_fast32_t *color;
                        if (pixel_state.fg_color != 0x00000000) {
                            color = &(pixel_state.fg_color);
                        } else {
                            color = &(pixel_state.bg_color);
                        }
                        int32_t biome = pixel_state.top_biome;
                        if (biome == 6 || biome == 134) {
                            *color = blend_color (*color, 0x665956ff, 0.5);
                        } else if (biome == 21 || biome == 149 || biome == 23 ||
                                   biome == 151) {
                            *color = blend_color (*color, 0x83bd7eff, 0.5);
                        } else if (biome == 35 || biome == 163) {
                            *color = blend_color (*color, 0xa8ab33ff, 0.5);
                        }
                    }
                }
            }
        }

        void handle_inclination (
            shared_ptr<array<PixelState, 256 * 256>> pixel_states) {
            for (int z = 0; z < 16; ++z) {
                for (int x = 1; x < 16; ++x) {
                    PixelState &left = get_pixel_state (pixel_states, x - 1, z);
                    PixelState &cur = get_pixel_state (pixel_states, x, z);
                    if (left.opaque_height < cur.opaque_height) {
                        cur.bg_color = increase_brightness (cur.bg_color, 30);
                        if (x == 1) {
                            left.bg_color =
                                increase_brightness (left.bg_color, 30);
                        }
                    } else if (cur.opaque_height < left.opaque_height) {
                        cur.bg_color = increase_brightness (cur.bg_color, -30);
                        if (x == 1) {
                            left.bg_color =
                                increase_brightness (left.bg_color, -30);
                        }
                    }
                }
            }

            for (int z = 1; z < 16; ++z) {
                for (int x = 0; x < 16; ++x) {
                    PixelState &cur = get_pixel_state (pixel_states, x, z);
                    PixelState &upper =
                        get_pixel_state (pixel_states, x, z - 1);

                    if (upper.opaque_height < cur.opaque_height) {
                        cur.bg_color = increase_brightness (cur.bg_color, 10);
                        if (z == 1) {
                            upper.bg_color =
                                increase_brightness (upper.bg_color, 10);
                        }
                    } else if (cur.opaque_height < upper.opaque_height) {
                        cur.bg_color = increase_brightness (cur.bg_color, -10);
                        if (z == 1) {
                            upper.bg_color =
                                increase_brightness (upper.bg_color, -10);
                        }
                    }
                }
            }
        }

        void process_pipeline (
            shared_ptr<array<PixelState, 256 * 256>> pixel_states) {
            handle_biomes (pixel_states);
            handle_inclination (pixel_states);
        }

        void
        generate_image (int chunk_x, int chunk_z,
                        shared_ptr<array<PixelState, 256 * 256>> pixel_states,
                        Png &image) {
            for (int z = 0; z < 16; ++z) {
                for (int x = 0; x < 16; ++x) {
                    PixelState &pixel_state =
                        get_pixel_state (pixel_states, x, z);

                    uint_fast32_t bg_color = increase_brightness (
                        pixel_state.mid_color,
                        (pixel_state.mid_height - pixel_state.top_height) * 3);
                    uint_fast32_t color =
                        blend_color (pixel_state.fg_color, bg_color);
                    bg_color = increase_brightness (
                        pixel_state.bg_color,
                        (pixel_state.opaque_height - pixel_state.top_height) *
                            3);
                    color = blend_color (color, bg_color);
                    image.set_pixel (chunk_x * 16 + x, chunk_z * 16 + z, color);
                }
            }
        }

        void generate_chunk (anvil::Chunk *chunk, int chunk_x, int chunk_z,
                             Png &image) {
            shared_ptr<array<PixelState, 256 * 256>> pixel_states =
                scan_chunk (chunk);
            process_pipeline (pixel_states);
            generate_image (chunk_x, chunk_z, pixel_states, image);
        }
    } // namespace

    void generate_256 (shared_ptr<QueuedItem> item) {
        anvil::Region *region = item->region->region;
        int region_x = item->region->rx;
        int region_z = item->region->rz;
        int off_x = item->off_x;
        int off_z = item->off_z;

        if (option_verbose) {
            logger::d ("generating "s + item->debug_string () + " ..."s);
        }

        filesystem::path path = option_out_dir;
        path /= (to_string (region_x * 2 + off_x) + ',' +
                 to_string (region_z * 2 + off_z) + ".png"s);

        Png *image = nullptr;

        /* minumum range of chunk update is radius of 3, so we can capture
           all updated chunk with step of 6. but, we set this 4 since
           4 can divide 16, our image (chunk) width. */
        for (int chunk_z = 0; chunk_z < 16; chunk_z += 4) {
            int prev_chunk_x = -1;
            for (int chunk_x = 0; chunk_x < 16; ++chunk_x) {
                anvil::Chunk *chunk;

                try {
                    chunk = region->get_chunk_if_dirty (off_x * 16 + chunk_x,
                                                        off_z * 16 + chunk_z);
                } catch (exception const &e) {
                    logger::e ("Warning: parse error in "s +
                               item->debug_string ());
                    logger::e (e.what ());
                    continue;
                }

                if (chunk == nullptr) {
                    continue;
                }

                if (image == nullptr) {
                    if (filesystem::exists (path)) {
                        try {
                            image = new Png (path.string ());

                        } catch (exception const &) {
                            image = new Png (256, 256);
                        }
                    } else {
                        image = new Png (256, 256);
                    }
                }

                generate_chunk (chunk, chunk_x, chunk_z, *image);

                delete chunk;

                int start_x =
                    chunk_x - 3 > prev_chunk_x ? chunk_x - 3 : prev_chunk_x + 1;
                int end_x = chunk_x + 4 > 16 ? 16 : chunk_x + 4;

                for (int t_chunk_x = start_x; t_chunk_x < end_x; ++t_chunk_x) {
                    for (int t_chunk_z = chunk_z + 1; t_chunk_z < chunk_z + 4;
                         ++t_chunk_z) {
                        try {
                            chunk = region->get_chunk_if_dirty (
                                off_x * 16 + t_chunk_x, off_z * 16 + t_chunk_z);
                        } catch (exception const &e) {
                            logger::e ("Warning: parse error in "s +
                                       item->debug_string ());
                            logger::e (e.what ());
                            continue;
                        }

                        if (chunk == nullptr) {
                            continue;
                        }

                        generate_chunk (chunk, t_chunk_x, t_chunk_z, *image);

                        delete chunk;
                    }
                }
                prev_chunk_x = chunk_x + 3;
            }
        }

        if (image == nullptr) {
            if (option_verbose) {
                logger::d (
                    "exiting without generating; any chunk changed in "s +
                    item->debug_string ());
            }

            return;
        }

        image->save (path.string ());
        delete image;

        if (option_verbose) {
            logger::d ("generated "s + item->debug_string ());
        }
    }

} // namespace generator
