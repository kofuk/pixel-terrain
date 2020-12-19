# SPDX-License-Identifier: MIT

add_library(mcregion STATIC chunk.cc region.cc utils.cc)
target_include_directories(mcregion PRIVATE SYSTEM ${ZLIB_INCLUDE_DIRS})
target_link_libraries(mcregion PRIVATE nbtpullparser)
target_link_libraries(mcregion PRIVATE ${ZLIB_MOD_NAME})

add_boost_test(nbt_utils_test nbt_utils_test utils_test.cc)
if(TARGET nbt_utils_test)
  target_link_libraries(nbt_utils_test mcregion)
endif()

add_subdirectory(pull_parser)
