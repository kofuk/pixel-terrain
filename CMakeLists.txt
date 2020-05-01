cmake_minimum_required(VERSION 3.15)
project(mcmap CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_STANDARD_REQUIRED ON)

find_program(CLANG_FOUND "clang++")
if(CLANG_FOUND)
  set(CMAKE_CXX_COMPILER "clang++")
endif()

find_package(ZLIB)
find_package(Threads)
find_package(Boost COMPONENTS unit_test_framework)
find_package(PkgConfig)
pkg_check_modules(LIBPNG REQUIRED libpng)

add_subdirectory(block)
add_subdirectory(src)
