srcs = [
  'chunk.cc',
  'nbt-path.cc',
  'nbt.cc',
  'region.cc',
  'tag.cc',
  'utils.cc'
]

mcregion_lib = static_library(
  'mcregion', srcs,
  include_directories : project_inc,
  dependencies : [zlib_dep])
