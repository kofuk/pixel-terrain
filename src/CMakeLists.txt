add_executable(mcmap main.cc)
add_dependencies(mcmap block_colors_data)
target_include_directories(mcmap PRIVATE SYSTEM ${ZLIB_INCLUDE_DIRS} ${LIBPNG_INCLUE_DIRS})
target_include_directories(mcmap PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(mcmap ${ZLIB_LIBRARIES} ${LIBPNG_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_subdirectory(functions)
add_subdirectory(logger)
add_subdirectory(nbt)

if(${CMAKE_CXX_COMPILER} STREQUAL "clang++")
  target_compile_options(mcmap PRIVATE -Wall -Wextra -O2)
else()
  target_compile_options(mcmap PRIVATE -Wall -Wextra -O2 --param=max-vartrack-size=55000000)
endif()

install(TARGETS mcmap DESTINATION bin)
