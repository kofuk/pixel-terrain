add_custom_target(block_colors_data
  BYPRODUCTS block_colors_data.hh
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gen_block_colors_data.sh block_colors_data.hh
  DEPENDS gen_block_colors_data.sh
  block_colors_a.tsv block_colors_b.tsv block_colors_c.tsv
  block_colors_d.tsv block_colors_e.tsv block_colors_f.tsv
  block_colors_g.tsv block_colors_h.tsv block_colors_i.tsv
  block_colors_j.tsv block_colors_k.tsv block_colors_l.tsv
  block_colors_m.tsv block_colors_n.tsv block_colors_o.tsv
  block_colors_p.tsv block_colors_q.tsv block_colors_r.tsv
  block_colors_s.tsv block_colors_t.tsv block_colors_v.tsv
  block_colors_w.tsv block_colors_y.tsv block_colors_z.tsv
  COMMENT "Embedding block color data"
  VERBATIM)
