add_custom_target(block_colors_data
  BYPRODUCTS block_colors_data.hh
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gen_block_colors_data.sh block_colors_data.hh
  DEPENDS gen_block_colors_data.sh
  minecraft/block_colors_a.tsv minecraft/block_colors_b.tsv minecraft/block_colors_c.tsv
  minecraft/block_colors_d.tsv minecraft/block_colors_e.tsv minecraft/block_colors_f.tsv
  minecraft/block_colors_g.tsv minecraft/block_colors_h.tsv minecraft/block_colors_i.tsv
  minecraft/block_colors_j.tsv minecraft/block_colors_k.tsv minecraft/block_colors_l.tsv
  minecraft/block_colors_m.tsv minecraft/block_colors_n.tsv minecraft/block_colors_o.tsv
  minecraft/block_colors_p.tsv minecraft/block_colors_q.tsv minecraft/block_colors_r.tsv
  minecraft/block_colors_s.tsv minecraft/block_colors_t.tsv minecraft/block_colors_v.tsv
  minecraft/block_colors_w.tsv minecraft/block_colors_y.tsv minecraft/block_colors_z.tsv
  COMMENT "Embedding block color data"
  VERBATIM)
