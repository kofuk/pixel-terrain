# SPDX-License-Identifier: MIT

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR})
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  add_definitions(-DOS_LINUX)
  set(OS_LINUX ON)
  set(OS_WIN OFF)
elseif(WIN32)
  add_definitions(-DOS_WIN)
  set(OS_LINUX OFF)
  set(OS_WIN ON)
endif()

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
  graphics
  mcregion
  logger
  pixtimage
  nbtpullparser
  worlds
  regetopt
  )

install(TARGETS pixel-terrain DESTINATION bin)

add_subdirectory(graphics)
add_subdirectory(image)
add_subdirectory(nbt)
add_subdirectory(logger)
add_subdirectory(utils)
add_subdirectory(worlds)

if(OS_LINUX)
  add_subdirectory(server)
  target_link_libraries(pixel-terrain PRIVATE pixtserver)
endif()

add_subdirectory(third_party/regetopt)
