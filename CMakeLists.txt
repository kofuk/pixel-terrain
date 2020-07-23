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

find_package(Threads REQUIRED)

if(PIXEL_TERRAIN_SUPER_BUILDING)
  include(${ZLIB_CONFIG_CMAKE})
  include(${PNG_CONFIG_CMAKE})
  include(${REGETOPT_CONFIG_CMAKE})

  set(ZLIB_MOD_NAME zlibstatic)
  set(PNG_MOD_NAME png_static)
else()
  find_package(ZLIB REQUIRED)
  find_package(PNG REQUIRED)
  if(NOT DEFINED REGETOPT_CONFIG_CMAKE)
    message(FATAL_ERROR "Must provide REGETOPT_CONFIG_CMAKE. (If you don't have it, execute build_regetopt.sh and build it.)")
  endif()
  include(${REGETOPT_CONFIG_CMAKE})

  set(ZLIB_MOD_NAME ZLIB::ZLIB)
  set(PNG_MOD_NAME PNG::PNG)
endif()
set(REGETOPT_MOD_NAME regetopt_static)

include(cmake/test_utils.cmake)

add_custom_target(generate_editor_aux DEPENDS
  ${PROJECT_SOURCE_DIR}/compile_flags.txt
  ${PROJECT_SOURCE_DIR}/src/.dir-locals.el)

get_target_property(ZLIB_INCLUDE_DIRS ${ZLIB_MOD_NAME} INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(PNG_INCLUDE_DIRS ${PNG_MOD_NAME} INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(REGETOPT_INCLUDE_DIRS ${REGETOPT_MOD_NAME} INTERFACE_INCLUDE_DIRECTORIES)
set(COMP_INCLUDE_DIRS "${ZLIB_INCLUDE_DIRS};${PNG_INCLUDE_DIRS};${REGETOPT_INCLUDE_DIRS};${CMAKE_BINARY_DIR}")
unset(REGETOPT_INCLUDE_DIRS)
unset(PNG_INCLUDE_DIRS)
unset(ZLIB_INCLUDE_DIRS)
add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/compile_flags.txt
  COMMAND tools/editor-aux/editor-aux compile_flags.txt ${PROJECT_SOURCE_DIR}/compile_flags.txt ${COMP_INCLUDE_DIRS}
  DEPENDS editor-aux
  COMMENT "Generating compile_flags.txt"
  VERBATIM)
add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/src/.dir-locals.el
  COMMAND  tools/editor-aux/editor-aux .dir-locals.el ${PROJECT_SOURCE_DIR}/src/.dir-locals.el ${COMP_INCLUDE_DIRS}
  DEPENDS editor-aux
  COMMENT "Generating src/.dir-locals.el"
  VERBATIM)
unset(COMP_INCLUDE_DIRS)

add_subdirectory(block)
add_subdirectory(src)
add_subdirectory(tools)
