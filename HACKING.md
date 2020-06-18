# Hacking Guide

## Building and Installing on your machine

### For GNU/Linux

Make sure the following required dependencies installed:

- CMake
- Make
- C++ compiler which supports ISO C++17
- Git

I recommend installing the following optional dependencies:

- libpng
- zlib
- Boost C++ library

libpng and zlib will be downloaded and built by build system if missing.
Boost is testing dependency; if you want run the tests for this project,
install Boost and run following 2 build rules: `test_executable` and `test`.

You can build and install by the following shell commands:

```
$ mkdir build; cd $_
$ cmake ..
$ make
$ sudo make install
```

### For Windows

Make sure C++ desktop app toolchain installed with Visual Studio on your machine.
You can optionally install Boost C++ library to run the tests.

This project can't be built on Visual Studio IDE due to CMake's ExternalProject configuration
and I won't support it for now. You have to build from Visual Studio's developer command-line
tools.

Even if you are using 64 bit machine, default "Developer PowerShell" is configured
to build 32 bit binary. You should choose x64 Native Tools Command Prompt from start menu.

Once launch the command line window, you can build by following shell commands:

```
> mkdir build
> cmake -G"NMake Makefiles" ..
> nmake
```

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
