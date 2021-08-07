project(
  'regetopt', 'c',
  default_options : ['warning_level=3'])

regetopt_inc = include_directories('include')

regetopt_lib = static_library(
  'regetopt', 'regetopt.c',
  include_directories : regetopt_inc,
  install : true)

regetopt_dep = declare_dependency(
  include_directories : regetopt_inc,
  link_with : regetopt_lib)
