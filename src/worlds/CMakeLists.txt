# SPDX-License-Identifier: MIT

add_library(worlds STATIC worlds.cc)
if(USE_V3_NBT_PARSER)
  target_link_libraries(worlds PRIVATE mcregion)
else()
  target_link_libraries(worlds PRIVATE nbtpullparser)
endif()
