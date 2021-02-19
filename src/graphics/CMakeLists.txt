# SPDX-License-Identifier: MIT

set(GRAPHICS_SRC
  color.cc
  png.cc)

add_library(graphics STATIC ${GRAPHICS_SRC})
target_include_directories(graphics PUBLIC SYSTEM ${PNG_INCLUDE_DIRS})
target_link_libraries(graphics PUBLIC ${PNG_MOD_NAME})

add_boost_test(color_test imagegen_color color_test.cc)
if(TARGET color_test)
  target_link_libraries(color_test graphics)
endif()
