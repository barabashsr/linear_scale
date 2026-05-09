#include "ui_styles.h"

lv_style_t style_bg, style_panel, style_title, style_value_large, style_value_medium;
lv_style_t style_sp_value, style_sp_active, style_btn, style_btn_danger;
lv_style_t style_label_small, style_conn_ok, style_conn_err;

void ui_styles_init(void)
{
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, CFG_LV_BG);
    lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);

    lv_style_init(&style_panel);
    lv_style_set_bg_color(&style_panel, CFG_LV_PANEL_BG);
    lv_style_set_bg_opa(&style_panel, LV_OPA_COVER);
    lv_style_set_border_width(&style_panel, CFG_UI_PANEL_BORDER);
    lv_style_set_border_color(&style_panel, CFG_LV_ACCENT);
    lv_style_set_radius(&style_panel, CFG_UI_PANEL_RADIUS);
    lv_style_set_pad_all(&style_panel, CFG_UI_PANEL_PAD);

    lv_style_init(&style_title);
    lv_style_set_text_color(&style_title, CFG_LV_TEXT);
    lv_style_set_text_font(&style_title, CFG_FONT_TITLE);

    lv_style_init(&style_value_large);
    lv_style_set_text_color(&style_value_large, CFG_LV_TEXT);
    lv_style_set_text_font(&style_value_large, CFG_FONT_MAIN_VALUE);

    lv_style_init(&style_value_medium);
    lv_style_set_text_color(&style_value_medium, CFG_LV_TEXT);
    lv_style_set_text_font(&style_value_medium, CFG_FONT_MAIN_LABEL);

    lv_style_init(&style_sp_value);
    lv_style_set_text_color(&style_sp_value, CFG_LV_TEXT_DIM);
    lv_style_set_text_font(&style_sp_value, CFG_FONT_SP_VALUE);

    lv_style_init(&style_sp_active);
    lv_style_set_text_color(&style_sp_active, CFG_LV_HIGHLIGHT);
    lv_style_set_text_font(&style_sp_active, CFG_FONT_SP_VALUE);

    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, CFG_LV_BTN_BG);
    lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_text_color(&style_btn, CFG_LV_TEXT);
    lv_style_set_text_font(&style_btn, CFG_FONT_BTN);
    lv_style_set_radius(&style_btn, CFG_UI_BTN_RADIUS);
    lv_style_set_pad_all(&style_btn, CFG_UI_BTN_PAD);

    lv_style_init(&style_btn_danger);
    lv_style_set_bg_color(&style_btn_danger, CFG_LV_CONFIRM_BG);
    lv_style_set_bg_opa(&style_btn_danger, LV_OPA_COVER);
    lv_style_set_text_color(&style_btn_danger, CFG_LV_TEXT);
    lv_style_set_text_font(&style_btn_danger, CFG_FONT_BTN);
    lv_style_set_radius(&style_btn_danger, CFG_UI_DANGER_RADIUS);
    lv_style_set_pad_all(&style_btn_danger, CFG_UI_DANGER_PAD);

    lv_style_init(&style_label_small);
    lv_style_set_text_color(&style_label_small, CFG_LV_TEXT_DIM);
    lv_style_set_text_font(&style_label_small, CFG_FONT_SMALL);

    lv_style_init(&style_conn_ok);
    lv_style_set_text_color(&style_conn_ok, CFG_LV_GREEN);
    lv_style_set_text_font(&style_conn_ok, CFG_FONT_CONN);

    lv_style_init(&style_conn_err);
    lv_style_set_text_color(&style_conn_err, CFG_LV_HIGHLIGHT);
    lv_style_set_text_font(&style_conn_err, CFG_FONT_CONN);
}
