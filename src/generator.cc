#include <filesystem>

#include "PNG.hh"
#include "blocks.hh"
#include "generator.hh"
#include "logger.hh"

namespace Generator {
    static constexpr int32_t PS_IS_TRANSPARENT = 1;

    static inline int32_t get_pixel_state (array<int32_t, 256 * 256> &pixel_state,
                                    int x, int y) {
        return pixel_state[y * 256 + x];
    }

    static inline void add_pixel_state (array<int32_t, 256 * 256> &pixel_state, int x,
                                 int y, int32_t flags) {
        pixel_state[y * 256 + x] |= flags;
    }

    static inline void generate_chunk (Anvil::Chunk *chunk, int chunk_x, int chunk_z,
                                Png &image) {
        int max_y = chunk->get_max_height ();
        if (option_nether) {
            if (max_y > 127) max_y = 127;
        }

        array<int32_t, 256 * 256> pixel_state;
        pixel_state.fill (0);
        for (int z = 0; z < 16; ++z) {
            int prev_x = -1;
            int prev_y = -1;

            for (int x = 0; x < 16; ++x) {
                image.clear (chunk_x * 16 + x, chunk_z * 16 + z);
                bool air_found = false;
                int cur_top_y = -1;
                string prev_block;

                for (int y = max_y; y >= 0; --y) {
                    unsigned char new_alpha;
                    string block;
                    try {
                        block = chunk->get_block (x, y, z);
                    } catch (exception const &e) {
                        Logger::e ("Warning: error occurred while obtaining "
                                   "block");
                        Logger::e (e.what ());

                        continue;
                    }

                    if (block == "air" || block == "cave_air" ||
                        block == "void_air") {
                        air_found = true;
                        if (y == 0) {
                            image.blend (chunk_x * 16 + x, chunk_z * 16 + z, 0,
                                         0, 0, 255);
                        }
                        continue;
                    }

                    if (option_nether && !air_found) {
                        if (y == 0) {
                            image.blend (chunk_x * 16 + x, chunk_z * 16 + z, 0,
                                         0, 0, 255);
                        }

                        continue;
                    }

                    if (block == prev_block) {
                        if (y == 0) {
                            image.blend (chunk_x * 16 + x, chunk_z * 16 + z, 0,
                                         0, 0, 255);
                        }
                        continue;
                    }

                    prev_block = block;

                    auto color_itr = colors.find (block);
                    if (color_itr == end (colors)) {
                        Logger::i (R"(colors[")" + block + R"("] = ???)");

                        new_alpha = image.blend (
                            chunk_x * 16 + x, chunk_z * 16 + z, 0, 0, 0, 255);
                    } else {
                        array<unsigned char, 4> color = color_itr->second;

                        new_alpha = image.blend (chunk_x * 16 + x,
                                                 chunk_z * 16 + z, color[0],
                                                 color[1], color[2], color[3]);

                        if (prev_y < 0 || prev_y == y) {
                            /* do nothing */
                        } else if (prev_y < y) {
                            image.increase_brightness (chunk_x * 16 + x,
                                                       chunk_z * 16 + z, 30);
                            if (x == 1 &&
                                !(get_pixel_state (pixel_state, x - 1, z) &
                                  PS_IS_TRANSPARENT)) {
                                image.increase_brightness (
                                    chunk_x * 16 + x - 1, chunk_z * 16 + z, 30);
                            }
                        } else {
                            image.increase_brightness (chunk_x * 16 + x,
                                                       chunk_z * 16 + z, -30);
                            if (x == 1 &&
                                !(get_pixel_state (pixel_state, x - 1, z) &
                                  PS_IS_TRANSPARENT)) {
                                image.increase_brightness (chunk_x * 16 + x - 1,
                                                           chunk_z * 16 + z,
                                                           -30);
                            }
                        }

                        if (cur_top_y == -1) {
                            cur_top_y = y;
                        }

                        if (new_alpha == 255 && cur_top_y != y) {
                            image.increase_brightness (chunk_x * 16 + x,
                                                       chunk_z * 16 + z,
                                                       (y - cur_top_y) * 2);
                            add_pixel_state (pixel_state, x, z,
                                             PS_IS_TRANSPARENT);
                        }
                    }

                    if (prev_x != x && new_alpha > 130) {
                        prev_x = x;
                        prev_y = y;
                    }

                    if (new_alpha < 255) continue;

                    break;
                }
            }
        }
    }

    void generate_256 (QueuedItem *item) {
        Anvil::Region *region = item->region->region;
        int region_x = item->region->rx;
        int region_z = item->region->rz;
        int off_x = item->off_x;
        int off_z = item->off_z;

        if (option_verbose) {
            Logger::d ("generating " + item->debug_string () + " ...");
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
                Anvil::Chunk *chunk;

                try {
                    chunk = region->get_chunk_if_dirty (off_x * 16 + chunk_x,
                                                        off_z * 16 + chunk_z);
                } catch (exception const &e) {
                    Logger::e ("Warning: parse error in " +
                               item->debug_string ());
                    Logger::e (e.what ());
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
                            Logger::e ("Warning: parse error in " +
                                       item->debug_string ());
                            Logger::e (e.what ());
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
                Logger::d ("exiting without generating; any chunk changed in " +
                           item->debug_string ());
            }

            return;
        }

        image->save (path.string ());
        delete image;

        if (option_verbose) {
            Logger::d ("generated " + item->debug_string ());
        }
    }

} // namespace Generator
