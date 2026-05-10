idf_component_register(
    SRCS "main.c"
         "lcd_port.c"
         "aw9523.c"
         "scale.c"
         "spindle_enc.c"
         "buttons.c"
         "ui_logic.c"
         "ui_styles.c"
         "ui_main.c"
         "fonts/ui_font_roboto_20.c"
         "fonts/ui_font_roboto_24.c"
         "fonts/ui_font_roboto_28.c"
         "fonts/ui_font_roboto_bold_36.c"
         "fonts/ui_font_roboto_bold_48.c"
         "fonts/ui_font_roboto_bold_56.c"
         "fonts/ui_font_roboto_bold_60.c"
         "fonts/ui_font_roboto_bold_72.c"
    INCLUDE_DIRS "."
                  "fonts"
    REQUIRES driver freertos esp_timer esp_lcd
)

idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
target_compile_options(${lvgl_lib} PRIVATE -Wno-format)
