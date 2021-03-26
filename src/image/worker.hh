// SPDX-License-Identifier: MIT

#ifndef WORKER_HH
#define WORKER_HH

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <set>
#include <string>

#include "graphics/png.hh"
#include "image/containers.hh"
#include "nbt/chunk.hh"
#include "nbt/constants.hh"

namespace pixel_terrain::image {
    class worker {
        class pixel_state {
            std::uint32_t flags_ = 0;
            unsigned int top_height_ = 0;
            unsigned int mid_height_ = 0;
            unsigned int opaque_height_ = 0;
            std::uint32_t fg_color_ = 0;
            std::uint32_t mid_color_ = 0;
            std::uint32_t bg_color_ = 0;
            std::int32_t top_biome_ = 0;
#if USE_BLOCK_LIGHT_DATA
            std::uint8_t block_light_ = 0;
#endif

        public:
            static constexpr std::int32_t IS_TRANSPARENT = 1;
            static constexpr std::int32_t BIOME_OVERRIDDEN = 1 << 1;

            void add_flags(std::int32_t flags) { this->flags_ |= flags; }

            [[nodiscard]] auto get_flag(std::int32_t field) const -> bool {
                return static_cast<bool>(this->flags_ & field);
            }

            void set_top_height(std::uint8_t top_height) {
                top_height_ = top_height;
            }

            [[nodiscard]] auto top_height() const -> std::uint8_t {
                return top_height_;
            }

            void set_mid_height(unsigned int mid_height) {
                mid_height_ = mid_height;
            }

            [[nodiscard]] auto mid_height() const -> unsigned int {
                return mid_height_;
            }

            void set_opaque_height(unsigned int opaque_height) {
                opaque_height_ = opaque_height;
            };

            [[nodiscard]] auto opaque_height() const -> unsigned int {
                return opaque_height_;
            }

            void set_fg_color(std::uint32_t color) { fg_color_ = color; }

            [[nodiscard]] auto fg_color() const -> std::uint32_t {
                return fg_color_;
            }

            void set_mid_color(std::uint32_t color) { mid_color_ = color; }

            [[nodiscard]] auto mid_color() const -> std::uint32_t {
                return mid_color_;
            }

            void set_bg_color(std::uint32_t color) { bg_color_ = color; }

            [[nodiscard]] auto bg_color() const -> std::uint32_t {
                return bg_color_;
            }

            void set_top_biome(std::int32_t biome) { top_biome_ = biome; }

            [[nodiscard]] auto top_biome() const -> std::int32_t {
                return top_biome_;
            }

#if USE_BLOCK_LIGHT_DATA
            void set_block_light(std::uint8_t level) { block_light_ = level; }

            [[nodiscard]] auto block_light() -> std::uint8_t {
                return block_light_;
            }
#endif
        };

        using pixel_states =
            std::array<pixel_state, nbt::biomes::CHUNK_WIDTH *
                                        nbt::biomes::CHUNK_PER_REGION_WIDTH *
                                        nbt::biomes::CHUNK_WIDTH *
                                        nbt::biomes::CHUNK_PER_REGION_WIDTH>;

        mutable std::mutex unknown_blocks_mutex_;
        mutable std::set<std::string> unknown_blocks_;

        static inline auto get_pixel_state(pixel_states *states, int x, int y)
            -> pixel_state & {
            return (*states)[y * nbt::biomes::CHUNK_WIDTH *
                                 nbt::biomes::CHUNK_PER_REGION_WIDTH +
                             x];
        }

        auto scan_chunk(anvil::chunk *chunk, options const &options) const
            -> pixel_states *;

        static void handle_biomes(pixel_states *pixel_states);

        static void handle_inclination(pixel_states *pixel_states);

#if USE_BLOCK_LIGHT_DATA
        static void handle_block_light(pixel_states *pixel_states);
#endif

        static void process_pipeline(pixel_states *pixel_states);

        static void generate_image(int chunk_x, int chunk_z,
                                   pixel_states *pixel_states,
                                   graphics::png &image);

        void generate_chunk(anvil::chunk *chunk, int chunk_x, int chunk_z,
                            graphics::png &image, options const &options) const;

    public:
        ~worker();
        void generate_region(region_container *item) const;
    };
} // namespace pixel_terrain::image

#endif
