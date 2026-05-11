idf_component_register(
    SRCS "main.c"
         "lcd_port.c"
         "aw9523.c"
         "scale.c"
         "spindle_enc.c"
         "buttons.c"
         "mcp23017.c"
         "keypad.c"
         "ui/ui_logic.c"
         "ui/ui_styles.c"
         "ui/ui_main.c"
         "ui/fonts/ui_font_roboto_20.c"
         "ui/fonts/ui_font_roboto_24.c"
         "ui/fonts/ui_font_roboto_28.c"
         "ui/fonts/ui_font_roboto_bold_48.c"
         "ui/fonts/ui_font_roboto_bold_60.c"
         "ui/fonts/ui_font_roboto_bold_72.c"
         "ui/fonts/ui_font_roboto_bold_diam_32.c"
         "ui/fonts/ui_font_bs_24.c"
    INCLUDE_DIRS "."
                  "ui"
                  "ui/fonts"
    REQUIRES driver freertos esp_timer esp_lcd
)

idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
target_compile_options(${lvgl_lib} PRIVATE -Wno-format)
