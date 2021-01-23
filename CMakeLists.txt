# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(pixel-terrain
  VERSION 2.2.0
  DESCRIPTION "Generate map image for your Minecraft worlds."
  HOMEPAGE_URL "https://github.com/kofuk/pixel-terrain"
  LANGUAGES C CXX)

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

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_CONTACT "Koki Fukuda <ko.fu.dev@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.14), libpng16-16 (>= 1.6), libstdc++6 (>= 5.2), zlib1g (>= 1.2)")
set(CPACK_DEBIAN_PACKAGE_SECTION "games")
include(CPack)

if(MSVC)
  set(OS_WIN Yes)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(OS_LINUX Yes)
endif()

enable_testing()
find_package(Boost COMPONENTS unit_test_framework)

add_custom_target(test_executable)

function(add_boost_test target_name test_name)
  if(NOT Boost_FOUND)
    return()
  endif()

  add_executable(${target_name} EXCLUDE_FROM_ALL ${ARGN})
  target_include_directories(${target_name} PRIVATE SYSTEM ${Boost_INCLUDE_DIRS})
  target_compile_definitions(${target_name} PRIVATE "BOOST_TEST_DYN_LINK=1")
  target_compile_definitions(${target_name} PRIVATE "BOOST_TEST_MAIN")
  target_link_libraries(${target_name} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

  add_dependencies(test_executable ${target_name})

  add_test(NAME ${test_name} COMMAND ${target_name})
endfunction()

include(CheckCXXSourceCompiles)
check_cxx_source_compiles("char const*a=__FILE_NAME__;" HAVE_FILE_NAME_MACRO)
if(NOT HAVE_FILE_NAME_MACRO)
  if(OS_LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} '-D__FILE_NAME__=\"$(notdir $(abspath $<))\"'")
  else()
    # MSVC defines filename-only __FILE__ macro
    add_definitions("-D__FILE_NAME__=__FILE__")
  endif()
endif()
unset(HAVE_FILE_NAME_MACRO)

add_subdirectory(block)
add_subdirectory(tools)
add_subdirectory(src)

if(${CMAKE_BUILD_TYPE} MATCHES Debug AND NOT MSVC)
  add_compile_options(-fsanitize=address -fsanitize=leak)
  add_link_options(-fsanitize=address -fsanitize=leak)
endif()

configure_file(src/config.h.in ${CMAKE_BINARY_DIR}/config.h)

if(UNIX)
  install(FILES completion/pixel_terrain DESTINATION share/bash_completion/completions)
endif()

if(UNIX)
  install(FILES LICENSE DESTINATION share/licenses/pixel-terrain)
endif()
