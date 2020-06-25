add_library(region_parser STATIC chunk.cc region.cc nbt.cc utils.cc)
target_include_directories(image_generator PRIVATE SYSTEM ${PROJECT_SOURCE_DIR}/external/include)

add_subdirectory(pull_parser)
