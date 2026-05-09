idf_component_register(
    SRCS "main.c"
         "waveshare_rgb_lcd_port.c"
         "lvgl_port.c"
         "reader_comm.c"
         "ui_logic.c"
         "ui_styles.c"
         "ui_main.c"
    INCLUDE_DIRS "."
                  "../../shared"
    REQUIRES driver freertos esp_timer esp_lcd
)

idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
target_compile_options(${lvgl_lib} PRIVATE -Wno-format)
