# SPDX-License-Identifier: MIT

set(NBT_TESTDATA_NAMES
  broken
  complex-1
  complex-2
  complex-3
  nbt-file
  nested-list
  simple-tag-compound
  single-tag-byte-array
  single-tag-byte
  single-tag-double
  single-tag-float
  single-tag-int-array
  single-tag-int
  single-tag-list-int
  single-tag-long-array
  single-tag-long
  single-tag-short
  single-tag-string
  tag-multiple)

add_custom_target(nbt_testdata)

foreach(testdata_name ${NBT_TESTDATA_NAMES})
  generate_binary(nbt_testdata_${testdata_name}
    ${CMAKE_CURRENT_BINARY_DIR}/${testdata_name}.nbt ${testdata_name}.txt)
  add_dependencies(nbt_testdata nbt_testdata_${testdata_name})
endforeach()

add_custom_target(nbt_testdata_header DEPENDS ${CMAKE_BINARY_DIR}/nbt_test_testdata.hh)
add_dependencies(nbt_testdata_header embed_files)

file(GLOB NBT_TESTDATA ${CMAKE_CURRENT_BINARY_DIR}/*.nbt)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/nbt_test_testdata.hh
  COMMENT "Generating nbt test data header..."
  COMMAND ${CMAKE_BINARY_DIR}/tools/embed_files
  ${CMAKE_BINARY_DIR}/nbt_test_testdata.hh
  ${NBT_TESTDATA}
  DEPENDS embed_files nbt_testdata
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM)

