add_executable(mcmap Chunk.cc NBT.cc PNG.cc Region.cc Trie.cc blocks.cc main.cc server.cc utils.cc worker.cc)
target_include_directories(mcmap PRIVATE ${ZLIB_INCLUDE_DIRS} ${LIBPNG_INCLUE_DIRS})
target_include_directories(mcmap PRIVATE ${PROJECT_SOURCE_DIR}/cpptoml/include)
target_link_libraries(mcmap ${ZLIB_LIBRARIES} ${LIBPNG_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS mcmap DESTINATION bin)
