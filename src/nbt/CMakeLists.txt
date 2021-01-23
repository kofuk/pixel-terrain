# SPDX-License-Identifier: MIT

set(REGION_SRCS
  chunk.cc
  nbt-path.cc
  nbt.cc
  region.cc
  tag.cc
  utils.cc)

add_library(mcregion STATIC ${REGION_SRCS})
target_include_directories(mcregion PRIVATE SYSTEM ${ZLIB_INCLUDE_DIRS})
target_link_libraries(mcregion PRIVATE nbtpullparser)
target_link_libraries(mcregion PRIVATE ${ZLIB_MOD_NAME})

add_boost_test(tag_test tag_parser tag_test.cc)
if(TARGET tag_test)
  add_dependencies(tag_test nbt_testdata_header)
  target_link_libraries(tag_test mcregion)
endif()

add_boost_test(nbt_path_test nbt_path_test nbt-path_test.cc)
if(TARGET nbt_path_test)
  target_link_libraries(nbt_path_test mcregion)
endif()

add_boost_test(nbt_test nbt_test nbt_test.cc)
if(TARGET nbt_test)
  target_link_libraries(nbt_test mcregion)
endif()

add_subdirectory(testdata)
add_subdirectory(pull_parser)
