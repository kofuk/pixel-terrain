add_executable(mcmap Chunk.cc NBT.cc PNG.cc Region.cc blocks.cc logger.cc main.cc server.cc utils.cc worker.cc)
target_include_directories(mcmap PRIVATE ${ZLIB_INCLUDE_DIRS} ${LIBPNG_INCLUE_DIRS})
target_include_directories(mcmap PRIVATE ${PROJECT_SOURCE_DIR}/cpptoml/include)
target_link_libraries(mcmap ${ZLIB_LIBRARIES} ${LIBPNG_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_compile_options(mcmap PRIVATE -Wall -Wextra)

install(TARGETS mcmap DESTINATION bin)
