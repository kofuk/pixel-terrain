# SPDX-License-Identifier: MIT

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(PIXEL_TERRAIN_SRCS
  pixel-terrain.cc
  block-server.cc
  dump-nbt.cc
  generate-image.cc
  nbt-to-xml.cc
  world-info.cc
  )

add_executable(pixel-terrain ${PIXEL_TERRAIN_SRCS})
add_dependencies(pixel-terrain
  block_colors_data
  )
target_link_libraries(pixel-terrain PRIVATE
  mcregion
  logger
  pixtimage
  pixtserver
  nbtpullparser
  worlds
  regetopt
  )
install(TARGETS pixel-terrain DESTINATION bin)

add_subdirectory(graphics)
add_subdirectory(image)
add_subdirectory(nbt)
add_subdirectory(server)
add_subdirectory(logger)
add_subdirectory(utils)
add_subdirectory(worlds)

add_subdirectory(third_party/regetopt)
