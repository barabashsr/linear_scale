#include "ui_styles.h"

lv_style_t style_bg;
lv_style_t style_panel;
lv_style_t style_title;
lv_style_t style_value_large;
lv_style_t style_value_medium;
lv_style_t style_sp_value;
lv_style_t style_sp_active;
lv_style_t style_btn;
lv_style_t style_btn_danger;
lv_style_t style_btn_active;
lv_style_t style_label_small;
lv_style_t style_conn_ok;
lv_style_t style_conn_err;

void ui_styles_init(void)
{
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, COLOR_BG);
    lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);

    lv_style_init(&style_panel);
    lv_style_set_bg_color(&style_panel, COLOR_PANEL_BG);
    lv_style_set_bg_opa(&style_panel, LV_OPA_COVER);
    lv_style_set_border_width(&style_panel, 1);
    lv_style_set_border_color(&style_panel, COLOR_ACCENT);
    lv_style_set_radius(&style_panel, 8);
    lv_style_set_pad_all(&style_panel, 6);

    lv_style_init(&style_title);
    lv_style_set_text_color(&style_title, COLOR_TEXT);
    lv_style_set_text_font(&style_title, &lv_font_montserrat_20);

    lv_style_init(&style_value_large);
    lv_style_set_text_color(&style_value_large, COLOR_TEXT);
    lv_style_set_text_font(&style_value_large, &lv_font_montserrat_48);

    lv_style_init(&style_value_medium);
    lv_style_set_text_color(&style_value_medium, COLOR_TEXT);
    lv_style_set_text_font(&style_value_medium, &lv_font_montserrat_24);

    lv_style_init(&style_sp_value);
    lv_style_set_text_color(&style_sp_value, COLOR_TEXT_DIM);
    lv_style_set_text_font(&style_sp_value, &lv_font_montserrat_20);

    lv_style_init(&style_sp_active);
    lv_style_set_text_color(&style_sp_active, COLOR_HIGHLIGHT);
    lv_style_set_text_font(&style_sp_active, &lv_font_montserrat_20);

    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, COLOR_BTN_BG);
    lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_text_color(&style_btn, COLOR_TEXT);
    lv_style_set_text_font(&style_btn, &lv_font_montserrat_16);
    lv_style_set_radius(&style_btn, 4);
    lv_style_set_pad_all(&style_btn, 4);

    lv_style_init(&style_btn_danger);
    lv_style_set_bg_color(&style_btn_danger, COLOR_CONFIRM_BG);
    lv_style_set_bg_opa(&style_btn_danger, LV_OPA_COVER);
    lv_style_set_text_color(&style_btn_danger, COLOR_TEXT);
    lv_style_set_text_font(&style_btn_danger, &lv_font_montserrat_16);
    lv_style_set_radius(&style_btn_danger, 4);
    lv_style_set_pad_all(&style_btn_danger, 4);

    lv_style_init(&style_label_small);
    lv_style_set_text_color(&style_label_small, COLOR_TEXT_DIM);
    lv_style_set_text_font(&style_label_small, &lv_font_montserrat_12);

    lv_style_init(&style_conn_ok);
    lv_style_set_text_color(&style_conn_ok, COLOR_GREEN);
    lv_style_set_text_font(&style_conn_ok, &lv_font_montserrat_14);

    lv_style_init(&style_conn_err);
    lv_style_set_text_color(&style_conn_err, COLOR_HIGHLIGHT);
    lv_style_set_text_font(&style_conn_err, &lv_font_montserrat_14);
}
