srcs = [
  'blocks.cc',
  'generator.cc',
  'utils.cc',
  'worker.cc',
  block_colors_data_header
]

pixtimage_lib = static_library(
  'pixtimage', srcs,
  link_with : graphics_lib,
  include_directories : project_inc,
  dependencies : threads_dep)
