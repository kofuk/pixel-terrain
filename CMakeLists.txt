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
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

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

include(CheckCXXSourceCompiles)
check_cxx_source_compiles("#define A(...)a(1 __VA_OPT__(,)__VA_ARGS__)
void a(...){}int main(){A();}" HAVE_VA_OPT)
if(HAVE_VA_OPT)
  add_definitions(-DHAVE_VA_OPT=1)
endif()

option(USE_V3_NBT_PARSER "Use experimental NBT parser")
if(USE_V3_NBT_PARSER)
  add_definitions(-DUSE_V3_NBT_PARSER=1)
else()
  add_definitions(-DUSE_V3_NBT_PARSER=0)
endif()

option(BUILD_TESTS "Enable testing")
if(BUILD_TESTS)
  find_package(Boost COMPONENTS unit_test_framework REQUIRED)
  enable_testing()
endif()

function(add_boost_test target_name test_name)
  if(NOT BUILD_TESTS)
    return()
  endif()

  add_executable(${target_name} ${ARGN})
  target_compile_definitions(${target_name} PRIVATE "BOOST_TEST_DYN_LINK=1")
  target_compile_definitions(${target_name} PRIVATE "BOOST_TEST_MAIN")
  target_link_libraries(${target_name} Boost::unit_test_framework)

  add_test(NAME ${test_name} COMMAND ${target_name})
endfunction()

if(${CMAKE_BUILD_TYPE} MATCHES [Dd][Ee][Bb][Uu][Gg] AND NOT MSVC)
  add_compile_options(-fsanitize=address,leak)
  add_link_options(-fsanitize=address,leak)
endif()

configure_file(src/config.h.in ${CMAKE_BINARY_DIR}/config.h)

if(UNIX)
  install(FILES completion/pixel_terrain DESTINATION share/bash_completion/completions)
endif()

if(UNIX)
  install(FILES LICENSE DESTINATION share/licenses/pixel-terrain)
endif()

add_subdirectory(block)
add_subdirectory(tools)
add_subdirectory(src)
