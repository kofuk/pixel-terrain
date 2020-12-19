# SPDX-License-Identifier: MIT

add_library(pixtimage STATIC blocks.cc generator.cc worker.cc)
target_include_directories(pixtimage PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(pixtimage PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(pixtimage PRIVATE graphics)
add_dependencies(pixtimage block_colors_data)
