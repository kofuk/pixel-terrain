# SPDX-License-Identifier: MIT
project('pixel-terrain', 'cpp',
        version : '2.2.0',
        default_options : [
          'warning_level=3',
          'cpp_std=c++20'])

# Dependencies
threads_dep = dependency('threads')
zlib_dep = dependency('zlib')
png_dep = dependency('libpng')

regetopt_proj = subproject('regetopt')
regetopt_dep = regetopt_proj.get_variable('regetopt_dep')

# Platform detection
if host_machine.system() == 'linux'
  add_project_arguments('-DOS_LINUX', language : 'cpp')
elif host_machine.system() == 'windows'
  add_project_arguments('-DOS_WIN', language : 'cpp')
endif

# Check __VA_OPT__ macro
cxx_compiler = meson.get_compiler('cpp')
code = '''#define A(...)a(1 __VA_OPT__(,)__VA_ARGS__)
void a(...){}int main(){A();}
'''
have_va_opt = cxx_compiler.compiles(code, name : '__VA_OPT__')
if have_va_opt
  add_project_arguments('-DHAVE_VA_OPT', language : 'cpp')
endif

# Build options
add_project_arguments(
  '-DUSE_V3_NBT_PARSER=@0@'.format(get_option('v3_nbt_parser').enabled().to_int()),
  language : 'cpp')
add_project_arguments(
  '-DUSE_BLOCK_LIGHT_DATA=@0@'.format(get_option('block_light').enabled().to_int()),
  language : 'cpp')

# Install bash-completion if needed
if host_machine.system() == 'linux'
  if get_option('bash_comp')
    install_data(
      'completion/pixel_terrain',
      install_dir : 'share/bash_completion/completions')
  endif
endif

# Subdirectories
subdir('tools')
subdir('block')
subdir('src')
