# Hacking Guide

## Build instruction

### Superbuild (Easiest)

In this way, all dependencies are built (instead of using libraries on your system)
and statically linked. Unless you have special reason for using system-installed
libraries, I recommend to build in this way.

Make sure the following required dependencies installed:

- CMake
- Make (or other build system, such as Ninja)
- C++ compiler which supports ISO C++17
- Git

I recommend installing the following optional dependencies:

- Boost C++ library (for unit test)

You can build this project by the following commands:

```shell
$ ./third_party/prepare_libs.sh
$ mkdir superbuild/build
$ cd superbuild/build
$ cmake ..
$ make
```

If you are on Windows, you may have to specify CMake generator (to avoid
Visual Studio project to be generated; which does not work).
You should be able to choose any single configuration build system.

If your system doesn't have commands to execute `prepare_libs.sh`,
you have to manually download tarballs (URLs are written in `third_party/dep`)
and apply patches in third_party directory.

### Using system-installed libraries (Advanced)

Make sure following libraries are installed and can be found with CMake:

- zlib
- libpng

First, you have to build `regetopt` library (command-line option parser) because
it can't be found usual system. I sure that all you have to do is executing `build_regetopt.sh`.
Arguments you pass to the script will be passed to CMake.

Then, configure your build with top level CMakeLists.txt
(with option `build_regetopt.sh` displays in the last line).

## Set up environment for LSP

This project has build rule to generate configuration files for
clangd and Emacs. Run `generate_editor_aux` rule.

## Adding new blocks

Edit TSV (Tab-Separated Values) files on `/block`.
These are stored in `/block/${namespace}/block_colors_${prefix}.tsv` for convenience.
If you add new namespace or new prefix, remember editing `CMakeLists.txt`.

Format for each TSV files is following:

```tsv
block_id	r	g	b	a
foo	ff	00	00	ff
```

Note that first line is reserved for index and always skipped.
Second line adds a red block `foo` with completely opaque.
You MUST write each component of color in hex.
