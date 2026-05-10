idf_component_register(
    SRCS "main.c"
         "lcd_port.c"
         "aw9523.c"
    INCLUDE_DIRS "."
    REQUIRES driver freertos esp_timer esp_lcd
)

idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
target_compile_options(${lvgl_lib} PRIVATE -Wno-format)
