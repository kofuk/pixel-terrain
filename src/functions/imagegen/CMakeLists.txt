target_sources(pixel-terrain PRIVATE blocks.cc color.cc generator.cc worker.cc PNG.cc)

add_boost_test(color_test imagegen_color color_test.cc color.cc)
