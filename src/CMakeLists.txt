add_executable(mcmap Chunk.cc NBT.cc PNG.cc Region.cc blocks.cc generator.cc logger.cc main.cc server.cc utils.cc worker.cc)
target_include_directories(mcmap PRIVATE ${ZLIB_INCLUDE_DIRS} ${LIBPNG_INCLUE_DIRS})
target_include_directories(mcmap PRIVATE ${PROJECT_SOURCE_DIR}/cpptoml/include)
target_link_libraries(mcmap ${ZLIB_LIBRARIES} ${LIBPNG_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

if(${CMAKE_CXX_COMPILER} STREQUAL "clang++")
  target_compile_options(mcmap PRIVATE -Wall -Wextra -O2)
else()
  target_compile_options(mcmap PRIVATE -Wall -Wextra -O2 --param=max-vartrack-size=55000000)
endif()

install(TARGETS mcmap DESTINATION bin)
