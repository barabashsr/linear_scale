#include "ui_styles.h"
#include "config.h"
#include "ui_fonts.h"

lv_style_t s_bg, s_panel, s_title, s_val_big, s_val_med, s_val_sp, s_val_act;
lv_style_t s_btn, s_btn_danger, s_small, s_green, s_red;

void styles_init(void)
{
    lv_style_init(&s_bg);
    lv_style_set_bg_color(&s_bg, CFG_LV_BG); lv_style_set_bg_opa(&s_bg, LV_OPA_COVER);

    lv_style_init(&s_panel);
    lv_style_set_bg_color(&s_panel, CFG_LV_PANEL_BG); lv_style_set_bg_opa(&s_panel, LV_OPA_COVER);
    lv_style_set_border_width(&s_panel, CFG_UI_PANEL_BORDER);
    lv_style_set_border_color(&s_panel, CFG_LV_ACCENT);
    lv_style_set_radius(&s_panel, CFG_UI_PANEL_RADIUS); lv_style_set_pad_all(&s_panel, CFG_UI_PANEL_PAD);

    lv_style_init(&s_title);
    lv_style_set_text_color(&s_title, CFG_LV_TEXT); lv_style_set_text_font(&s_title, CFG_FONT_TITLE);

    lv_style_init(&s_val_big);
    lv_style_set_text_color(&s_val_big, CFG_LV_TEXT); lv_style_set_text_font(&s_val_big, CFG_FONT_MAIN_VALUE);

    lv_style_init(&s_val_med);
    lv_style_set_text_color(&s_val_med, CFG_LV_TEXT); lv_style_set_text_font(&s_val_med, CFG_FONT_MAIN_LABEL);

    lv_style_init(&s_val_sp);
    lv_style_set_text_color(&s_val_sp, CFG_LV_TEXT_DIM); lv_style_set_text_font(&s_val_sp, CFG_FONT_SP_VALUE);

    lv_style_init(&s_val_act);
    lv_style_set_text_color(&s_val_act, CFG_LV_HIGHLIGHT); lv_style_set_text_font(&s_val_act, CFG_FONT_SP_VALUE);

    lv_style_init(&s_btn);
    lv_style_set_bg_color(&s_btn, CFG_LV_BTN_BG); lv_style_set_bg_opa(&s_btn, LV_OPA_COVER);
    lv_style_set_text_color(&s_btn, CFG_LV_TEXT); lv_style_set_text_font(&s_btn, CFG_FONT_BTN);
    lv_style_set_radius(&s_btn, CFG_UI_BTN_RADIUS); lv_style_set_pad_all(&s_btn, CFG_UI_BTN_PAD);

    lv_style_init(&s_btn_danger);
    lv_style_set_bg_color(&s_btn_danger, CFG_LV_CONFIRM_BG); lv_style_set_bg_opa(&s_btn_danger, LV_OPA_COVER);
    lv_style_set_text_color(&s_btn_danger, CFG_LV_TEXT); lv_style_set_text_font(&s_btn_danger, CFG_FONT_BTN);
    lv_style_set_radius(&s_btn_danger, CFG_UI_DANGER_RADIUS); lv_style_set_pad_all(&s_btn_danger, CFG_UI_DANGER_PAD);

    lv_style_init(&s_small);
    lv_style_set_text_color(&s_small, CFG_LV_TEXT_DIM); lv_style_set_text_font(&s_small, CFG_FONT_SMALL);

    lv_style_init(&s_green);
    lv_style_set_text_color(&s_green, CFG_LV_GREEN); lv_style_set_text_font(&s_green, CFG_FONT_CONN);

    lv_style_init(&s_red);
    lv_style_set_text_color(&s_red, CFG_LV_HIGHLIGHT); lv_style_set_text_font(&s_red, CFG_FONT_CONN);
}
