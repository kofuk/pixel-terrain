# SPDX-License-Identifier: MIT

set(PIXTIMAGE_SRCS
  blocks.cc
  generator.cc
  utils.cc
  worker.cc
  )

add_library(pixtimage STATIC ${PIXTIMAGE_SRCS})
target_include_directories(pixtimage PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(pixtimage PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(pixtimage PRIVATE graphics)
add_dependencies(pixtimage block_colors_data)

add_boost_test(utils_test imagegen_utils utils_test.cc)
if(TARGET utils_test)
  target_link_libraries(utils_test pixtimage)
endif()
