cmake_minimum_required(VERSION 3.15)
project(pixel-terrain CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_STANDARD_REQUIRED ON)

include(ExternalProject)
ExternalProject_Add(optlib_project
  GIT_REPOSITORY https://github.com/kofuk/optlib.git
  GIT_TAG 941f5f7
  TEST_BEFORE_INSTALL YES
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/external -DCMAKE_BUILD_TYPE=Release)
add_library(optlib IMPORTED STATIC)
if(MSVC)
  set_target_properties(optlib PROPERTIES
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/external/lib/optlib.lib)
else()
  set_target_properties(optlib PROPERTIES
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/external/lib/liboptlib.a)
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
add_subdirectory(tools/editor-aux)
