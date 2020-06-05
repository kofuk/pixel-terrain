add_executable(block_data_generator block_data_generator.cc)

add_boost_test(block_data_generator_test blocks_generator_test block_data_generator.cc)
target_compile_definitions(block_data_generator_test PRIVATE -DTEST)

add_custom_target(block_colors_data DEPENDS block_colors_data.hh)

file(GLOB COLOR_DATA_SOURCES minecraft/*.tsv)

add_custom_command(OUTPUT block_colors_data.hh
  COMMAND block_data_generator ${CMAKE_CURRENT_BINARY_DIR}/block_colors_data.hh ${COLOR_DATA_SOURCES}
  DEPENDS block_data_generator ${COLOR_DATA_SOURCES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Embedding block color data"
  VERBATIM)
