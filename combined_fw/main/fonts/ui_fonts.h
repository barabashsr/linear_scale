/**
 * @file ui_fonts.h
 * @brief Custom Cyrillic fonts for Welding Positioner UI
 */

#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Font declarations - generated with lv_font_conv */
/* Include Cyrillic (0x410-0x44F, 0x401, 0x451) + Latin (0x20-0x7F) */

LV_FONT_DECLARE(ui_font_roboto_20);
LV_FONT_DECLARE(ui_font_roboto_24);
LV_FONT_DECLARE(ui_font_roboto_28);
LV_FONT_DECLARE(ui_font_roboto_bold_36);
LV_FONT_DECLARE(ui_font_roboto_bold_48);
LV_FONT_DECLARE(ui_font_roboto_bold_56);
LV_FONT_DECLARE(ui_font_roboto_bold_60);
LV_FONT_DECLARE(ui_font_roboto_bold_72);
LV_FONT_DECLARE(ui_font_roboto_bold_diam_32);

/* Font aliases for semantic usage */
#define UI_FONT_SMALL        &ui_font_roboto_20
#define UI_FONT_LABEL        &ui_font_roboto_24
#define UI_FONT_TITLE        &ui_font_roboto_28
#define UI_FONT_SETPOINT     &ui_font_roboto_bold_48
#define UI_FONT_MAIN         &ui_font_roboto_bold_72
#define UI_FONT_LARGE        &ui_font_roboto_bold_60
#define UI_FONT_HUGE         &ui_font_roboto_bold_72
#define UI_FONT_DIAM         &ui_font_roboto_bold_diam_32

#ifdef __cplusplus
}
#endif
