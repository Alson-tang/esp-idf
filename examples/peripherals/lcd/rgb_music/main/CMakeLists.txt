file(GLOB_RECURSE IMAGE_SOURCES ./music/assets/*.c)

idf_component_register(SRCS "rgb_lcd_example_main.c" 
                              "./music/lv_demo_music_list.c" 
                              "./music/lv_demo_music_main.c" 
                              "./music/lv_demo_music.c" 
                              ${IMAGE_SOURCES}
                       INCLUDE_DIRS "." 
                                   "./music" 
                                   "./music/assets") 

target_compile_options(${COMPONENT_LIB} PRIVATE -pg)
