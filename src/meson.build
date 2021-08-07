config_data = configuration_data(
  {
    'project_name': meson.project_name(),
    'version': meson.project_version()
  })
configure_file(
  input : 'config.h.in',
  output : 'config.h',
  configuration : config_data)

project_inc = include_directories('.')

subdir('logger')
subdir('nbt')
subdir('worlds')
subdir('graphics')
subdir('image')
subdir('server')

srcs = [
  'pixel-terrain.cc',
  'block-server.cc',
  'dump-nbt.cc',
  'generate-image.cc',
  'nbt-to-xml.cc',
  'world-info.cc',
]

executable(
  'pixel-terrain', srcs,
  link_with : [
    graphics_lib,
    mcregion_lib,
    logger_lib,
    pixtimage_lib,
    server_lib,
    worlds_lib],
  dependencies : regetopt_dep,
  install : true)
