#pragma once

#include "lvgl.h"

#define COLOR_BG             lv_color_hex(0x1A1A2E)
#define COLOR_PANEL_BG       lv_color_hex(0x16213E)
#define COLOR_ACCENT         lv_color_hex(0x0F3460)
#define COLOR_HIGHLIGHT      lv_color_hex(0xE94560)
#define COLOR_TEXT           lv_color_hex(0xEEEEEE)
#define COLOR_TEXT_DIM       lv_color_hex(0x888888)
#define COLOR_GREEN           lv_color_hex(0x2ECC71)
#define COLOR_BTN_BG          lv_color_hex(0x1A3A5C)
#define COLOR_BTN_ACTIVE_BG  lv_color_hex(0xE94560)
#define COLOR_CONFIRM_BG     lv_color_hex(0xC0392B)
#define COLOR_CANCEL_BG      lv_color_hex(0x7F8C8D)

extern lv_style_t style_bg;
extern lv_style_t style_panel;
extern lv_style_t style_title;
extern lv_style_t style_value_large;
extern lv_style_t style_value_medium;
extern lv_style_t style_sp_value;
extern lv_style_t style_sp_active;
extern lv_style_t style_btn;
extern lv_style_t style_btn_danger;
extern lv_style_t style_btn_active;
extern lv_style_t style_label_small;
extern lv_style_t style_conn_ok;
extern lv_style_t style_conn_err;

void ui_styles_init(void);
