add_executable(gen_map Chunk.cc NBT.cc Region.cc Utils.cc blocks.cc main.cc)
target_include_directories(gen_map PRIVATE ${ZLIB_INCLUDE_DIRS} ${LIBPNG_INCLUE_DIRS})
target_link_libraries(gen_map ${ZLIB_LIBRARIES} ${LIBPNG_LIBRARIES})
