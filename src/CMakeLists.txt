add_executable(mcmap Chunk.cc NBT.cc Region.cc Utils.cc blocks.cc main.cc worker.cc)
target_include_directories(mcmap PRIVATE ${ZLIB_INCLUDE_DIRS} ${LIBPNG_INCLUE_DIRS})
target_link_libraries(mcmap ${ZLIB_LIBRARIES} ${LIBPNG_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
