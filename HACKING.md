# Hacking Guide

## Formatting code

This project uses clang-format to format source files.

## Set up environment for LSP

This project has build rule to generate configuration files for
clangd and Emacs. Run `generate_editor_aux` rule.

## Adding new blocks

Edit TSV (Tab-Separated Values) files on `/block`.
These are stored in `/block/${namespace}/block_colors_${prefix}.tsv` for convenience.
If you add new namespace or new prefix, remember editing `CMakeLists.txt`.

Format for each TSV files is the following:

```tsv
block_id	r	g	b	a
foo	ff	00	00	ff
```

Note that first line is reserved for index and always skipped.
Second line adds a red block `foo` with completely opaque.
You MUST write each component of color in hex.
