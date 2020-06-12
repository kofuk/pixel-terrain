target_sources(pixel-terrain PRIVATE blocks.cc color.cc generate_main.cc generator.cc worker.cc png.cc)

add_boost_test(color_test imagegen_color color_test.cc color.cc)
