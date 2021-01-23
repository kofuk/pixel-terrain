# SPDX-License-Identifier: MIT

add_executable(block_data_generator EXCLUDE_FROM_ALL block_data_generator.cc)

add_boost_test(block_data_generator_test blocks_generator_test block_data_generator.cc)
if(TARGET block_data_generator_test)
  target_compile_definitions(block_data_generator_test PRIVATE -DTEST)
endif()

add_executable(binary_generator EXCLUDE_FROM_ALL binary_generator.cc embed_files.cc)

function(generate_binary_header target_name output)
  get_filename_component(GEN_BINARY_OUT_FILENAME ${output} NAME)
  add_custom_target(${target_name} DEPENDS ${output})
  add_custom_command(OUTPUT ${output}
    COMMENT "Generating ${GEN_BINARY_OUT_FILENAME}..."
    COMMAND ${CMAKE_BINARY_DIR}/tools/binary_generator ${output} ${ARGN}
    DEPENDS binary_generator ${ARGN}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM)
  unset(GEN_BINARY_OUT_FILENAME)
endfunction()
