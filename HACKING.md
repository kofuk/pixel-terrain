# Hacking Guide

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
