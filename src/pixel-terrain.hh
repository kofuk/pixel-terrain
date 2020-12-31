// SPDX-License-Identifier: MIT

#ifndef PIXEL_TERRAIN_HH
#define PIXEL_TERRAIN_HH

namespace pixel_terrain {
    auto image_main(int argc, char **argv) -> int;
    auto dump_nbt_main(int argc, char **argv) -> int;
    auto nbt_to_xml_main(int argc, char **argv) -> int;
    auto server_main(int argc, char **argv) -> int;
    auto world_info_main(int argc, char **argv) -> int;
} // namespace pixel_terrain

#endif
