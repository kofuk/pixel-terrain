#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>

#include "PNG.hh"
#include "blocks.hh"
#include "color.hh"
#include "generator.hh"
#include "logger.hh"

namespace generator {
    struct PixelState {
        uint_fast32_t flags;
        uint_fast8_t top_height;
        uint_fast8_t mid_height;
        uint_fast8_t opaque_height;
        uint_fast32_t fg_color;
        uint_fast32_t mid_color;
        uint_fast32_t bg_color;

        PixelState ()
            : flags (0), top_height (0), mid_height (0), opaque_height (0),
              fg_color (0x00000000), mid_color (0x00000000),
              bg_color (0x00000000) {}
        void add_flags (int_fast32_t flags) { this->flags |= flags; }
        bool get_flag (int_fast32_t field) { return this->flags & field; }
    };

    static constexpr int32_t PS_IS_TRANSPARENT = 1;

    static inline PixelState &
    get_pixel_state (shared_ptr<array<PixelState, 256 * 256>> pixel_state,
                     int x, int y) {
        return (*pixel_state)[y * 256 + x];
    }

    static shared_ptr<array<PixelState, 256 * 256>>
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

                PixelState &pixel_state = get_pixel_state (pixel_states, x, z);

                for (int y = max_y; y >= 0; --y) {
                    string block;
                    try {
                        block = chunk->get_block (x, y, z);
                    } catch (exception const &e) {
                        logger::e ("Warning: error occurred while obtaining "
                                   "block");
                        logger::e (e.what ());

                        continue;
                    }

                    if (block == "air" || block == "cave_air" ||
                        block == "void_air") {
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
                        logger::i (R"(colors[")" + block + R"("] = ???)");
                    } else {
                        uint_fast32_t color = color_itr->second;

                        if (pixel_state.fg_color == 0x00000000) {
                            pixel_state.fg_color = color;
                            pixel_state.top_height = y;
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

    static void
    generate_image (int chunk_x, int chunk_z,
                    shared_ptr<array<PixelState, 256 * 256>> pixel_states,
                    Png &image) {
        for (int z = 0; z < 16; ++z) {
            for (int x = 1; x < 16; ++x) {
                PixelState &left = get_pixel_state (pixel_states, x - 1, z);
                PixelState &cur = get_pixel_state (pixel_states, x, z);
                if (left.opaque_height < cur.opaque_height) {
                    cur.bg_color = increase_brightness (cur.bg_color, 30);
                    if (x == 1) {
                        left.bg_color = increase_brightness (left.bg_color, 30);
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
                PixelState &upper = get_pixel_state (pixel_states, x, z - 1);

                if (upper.opaque_height < cur.opaque_height) {
                    cur.bg_color = increase_brightness (cur.bg_color, 10);
                    if (x == 1) {
                        upper.bg_color =
                            increase_brightness (upper.bg_color, 10);
                    }
                } else if (cur.opaque_height < upper.opaque_height) {
                    cur.bg_color = increase_brightness (cur.bg_color, -10);
                    if (x == 1) {
                        upper.bg_color =
                            increase_brightness (upper.bg_color, -10);
                    }
                }
            }
        }

        for (int z = 0; z < 16; ++z) {
            for (int x = 0; x < 16; ++x) {
                PixelState &pixel_state = get_pixel_state (pixel_states, x, z);

                uint_fast32_t bg_color = increase_brightness (
                    pixel_state.mid_color,
                    (pixel_state.mid_height - pixel_state.top_height) * 3);
                uint_fast32_t color =
                    blend_color (pixel_state.fg_color, bg_color);
                bg_color = increase_brightness (
                    pixel_state.bg_color,
                    (pixel_state.opaque_height - pixel_state.top_height) * 3);
                color = blend_color (color, bg_color);
                image.set_pixel (chunk_x * 16 + x, chunk_z * 16 + z, color);
            }
        }
    }

    static void generate_chunk (anvil::Chunk *chunk, int chunk_x, int chunk_z,
                                Png &image) {
        shared_ptr<array<PixelState, 256 * 256>> pixel_states =
            scan_chunk (chunk);
        generate_image (chunk_x, chunk_z, pixel_states, image);
    }

    void generate_256 (QueuedItem *item) {
        anvil::Region *region = item->region->region;
        int region_x = item->region->rx;
        int region_z = item->region->rz;
        int off_x = item->off_x;
        int off_z = item->off_z;

        if (option_verbose) {
            logger::d ("generating " + item->debug_string () + " ...");
        }

        filesystem::path path = option_out_dir;
        path /= (to_string (region_x * 2 + off_x) + ',' +
                 to_string (region_z * 2 + off_z) + ".png");

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
                    logger::e ("Warning: parse error in " +
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
                            logger::e ("Warning: parse error in " +
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
                logger::d ("exiting without generating; any chunk changed in " +
                           item->debug_string ());
            }

            return;
        }

        image->save (path.string ());
        delete image;

        if (option_verbose) {
            logger::d ("generated " + item->debug_string ());
        }
    }

} // namespace generator
