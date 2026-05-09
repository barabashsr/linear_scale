#pragma once

#include "config.h"
#include "lvgl.h"

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
