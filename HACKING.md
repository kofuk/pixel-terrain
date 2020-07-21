# Hacking Guide

## Build instruction

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
you have to manually download tarballs (URLs are written in the script file)
and apply patches in third_party directory.

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
