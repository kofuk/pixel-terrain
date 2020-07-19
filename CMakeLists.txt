# Copyright (c) 2020 Koki Fukuda
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.15)
project(pixel-terrain CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_STANDARD_REQUIRED ON)

include(ExternalProject)
ExternalProject_Add(regetopt_project
  GIT_REPOSITORY https://github.com/kofuk/regetopt.git
  GIT_TAG v0.0.1
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/external -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
add_library(regetopt IMPORTED STATIC)
if(MSVC)
  set_target_properties(regetopt PROPERTIES
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/external/lib/regetopt_static.lib)
else()
  set_target_properties(regetopt PROPERTIES
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/external/lib/libregetopt_static.a)
endif()

include(cmake/require_zlib.cmake)
include(cmake/require_png.cmake)
find_package(Threads REQUIRED)

include(cmake/test_utils.cmake)

add_custom_target(generate_editor_aux DEPENDS
  ${PROJECT_SOURCE_DIR}/compile_flags.txt
  ${PROJECT_SOURCE_DIR}/src/.dir-locals.el)

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/compile_flags.txt
  COMMAND tools/editor-aux/editor-aux compile_flags.txt ${PROJECT_SOURCE_DIR}/compile_flags.txt
  ${CMAKE_SOURCE_DIR}/external/include ${ZLIB_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}
  DEPENDS editor-aux
  COMMENT "Generating compile_flags.txt"
  VERBATIM)
add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/src/.dir-locals.el
  COMMAND  tools/editor-aux/editor-aux .dir-locals.el ${PROJECT_SOURCE_DIR}/src/.dir-locals.el
  ${CMAKE_SOURCE_DIR}/external/include ${ZLIB_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}
  DEPENDS editor-aux
  COMMENT "Generating src/.dir-locals.el"
  VERBATIM)

add_subdirectory(block)
add_subdirectory(src)
add_subdirectory(tools)
