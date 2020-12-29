# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(pixel-terrain C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

if(PIXEL_TERRAIN_SUPER_BUILDING)
  include(${ZLIB_CONFIG_CMAKE})
  include(${PNG_CONFIG_CMAKE})

  set(ZLIB_MOD_NAME zlibstatic)
  set(PNG_MOD_NAME png_static)
else()
  find_package(ZLIB REQUIRED)
  find_package(PNG REQUIRED)

  set(ZLIB_MOD_NAME ZLIB::ZLIB)
  set(PNG_MOD_NAME PNG::PNG)
endif()
set(REGETOPT_MOD_NAME regetopt_static)

if(MSVC)
  set(OS_WIN Yes)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(OS_LINUX Yes)
endif()

include(cmake/test_utils.cmake)

add_subdirectory(block)
add_subdirectory(src)
add_subdirectory(tools)

if(${CMAKE_BUILD_TYPE} MATCHES Debug AND NOT MSVC)
  add_compile_options(-fsanitize=address -fsanitize=leak)
  add_link_options(-fsanitize=address -fsanitize=leak)
endif()

add_custom_target(generate_editor_aux DEPENDS
  ${PROJECT_SOURCE_DIR}/compile_flags.txt
  ${PROJECT_SOURCE_DIR}/src/.dir-locals.el)

if(OS_LINUX)
  set(EDITOR_PLATFORM "Linux")
elseif(OS_WIN)
  set(EDITOR_PLATFORM "Windows")
endif()

get_target_property(ZLIB_INCLUDE_DIRS ${ZLIB_MOD_NAME} INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(PNG_INCLUDE_DIRS ${PNG_MOD_NAME} INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(REGETOPT_INCLUDE_DIRS regetopt INTERFACE_INCLUDE_DIRECTORIES)
set(COMP_INCLUDE_DIRS "${ZLIB_INCLUDE_DIRS};${PNG_INCLUDE_DIRS};${REGETOPT_INCLUDE_DIRS};${CMAKE_BINARY_DIR};${PROJECT_SOURCE_DIR}/src")
unset(REGETOPT_INCLUDE_DIRS)
unset(PNG_INCLUDE_DIRS)
unset(ZLIB_INCLUDE_DIRS)
add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/compile_flags.txt
  COMMENT "Generating compile_flags.txt"
  COMMAND tools/editor-aux/editor-aux
  ${EDITOR_PLATFORM}
  compile_flags.txt
  ${PROJECT_SOURCE_DIR}/compile_flags.txt
  ${COMP_INCLUDE_DIRS}
  DEPENDS editor-aux
  VERBATIM)

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/src/.dir-locals.el
  COMMENT "Generating src/.dir-locals.el"
  COMMAND tools/editor-aux/editor-aux
  ${EDITOR_PLATFORM}
  .dir-locals.el
  ${PROJECT_SOURCE_DIR}/src/.dir-locals.el
  ${COMP_INCLUDE_DIRS}
  DEPENDS editor-aux
  VERBATIM)
unset(COMP_INCLUDE_DIRS)

if(UNIX)
  install(FILES completion/pixel_terrain DESTINATION share/bash_completion/completions)
endif()

if(UNIX)
  install(FILES LICENSE DESTINATION share/licenses/pixel-terrain)
endif()
