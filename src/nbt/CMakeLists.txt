add_library(region_parser STATIC chunk.cc region.cc utils.cc)
target_include_directories(region_parser PRIVATE SYSTEM ${PROJECT_SOURCE_DIR}/external/include)
target_include_directories(region_parser PRIVATE SYSTEM ${ZLIB_INCLUDE_DIRS})
target_link_libraries(region_parser PRIVATE nbt_pull_parser)
target_link_libraries(region_parser INTERFACE ${ZLIB_LIBRARIES})

add_subdirectory(pull_parser)
