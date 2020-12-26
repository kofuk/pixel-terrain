// SPDX-License-Identifier: MIT

#ifndef WORKER_HH
#define WORKER_HH

#include <array>
#include <filesystem>
#include <memory>

#include "graphics/png.hh"
#include "image/containers.hh"
#include "nbt/chunk.hh"

namespace pixel_terrain::image {
    class worker {
        struct pixel_state {
            static constexpr std::int32_t IS_TRANSPARENT = 1;
            static constexpr std::int32_t BIOME_OVERRIDDEN = 1 << 1;

            std::uint_fast32_t flags;
            std::uint_fast8_t top_height;
            std::uint_fast8_t mid_height;
            std::uint_fast8_t opaque_height;
            std::uint_fast32_t fg_color;
            std::uint_fast32_t mid_color;
            std::uint_fast32_t bg_color;
            std::int32_t top_biome;

            pixel_state()
                : flags(0), top_height(0), mid_height(0), opaque_height(0),
                  fg_color(0x00000000), mid_color(0x00000000),
                  bg_color(0x00000000), top_biome(0) {}
            void add_flags(std::int_fast32_t flags) { this->flags |= flags; }
            bool get_flag(std::int_fast32_t field) {
                return this->flags & field;
            }
        };

        using pixel_states = std::array<pixel_state, 512 * 512>;

        options options_;

        static inline pixel_state &
        get_pixel_state(std::shared_ptr<pixel_states> pixel_state, int x,
                        int y) {
            return (*pixel_state)[y * 512 + x];
        }

        std::shared_ptr<pixel_states> scan_chunk(anvil::chunk *chunk) const;

        void handle_biomes(std::shared_ptr<pixel_states> pixel_states) const;

        void
        handle_inclination(std::shared_ptr<pixel_states> pixel_states) const;

        void process_pipeline(std::shared_ptr<pixel_states> pixel_states) const;

        void generate_image(int chunk_x, int chunk_z,
                            std::shared_ptr<pixel_states> pixel_states,
                            png &image) const;

        void generate_chunk(anvil::chunk *chunk, int chunk_x, int chunk_z,
                            png &image) const;

    public:
        worker(options opt) : options_(opt) {}
        void generate_region(std::shared_ptr<region_container> item) const;
    };
} // namespace pixel_terrain::image

#endif
