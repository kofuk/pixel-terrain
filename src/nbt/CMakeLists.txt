add_library(region_parser STATIC chunk.cc region.cc nbt.cc utils.cc)
target_include_directories(image_generator PRIVATE SYSTEM ${PROJECT_SOURCE_DIR}/external/include)
target_include_directories(block_server PRIVATE SYSTEM ${ZLIB_INCLUDE_DIRS})

add_subdirectory(pull_parser)
