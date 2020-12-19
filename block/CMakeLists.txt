# SPDX-License-Identifier: MIT

add_custom_target(block_colors_data DEPENDS block_colors_data.hh)
add_dependencies(block_colors_data block_data_generator)

file(GLOB COLOR_DATA_SOURCES minecraft/*.tsv)
add_custom_command(OUTPUT block_colors_data.hh
  COMMAND ${CMAKE_BINARY_DIR}/tools/block_data_generator ${CMAKE_CURRENT_BINARY_DIR}/block_colors_data.hh ${COLOR_DATA_SOURCES}
  DEPENDS block_data_generator ${COLOR_DATA_SOURCES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Embedding block color data"
  VERBATIM)
