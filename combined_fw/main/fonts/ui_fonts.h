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

/* Font aliases for semantic usage */
#define UI_FONT_LABEL       &ui_font_roboto_20     /* Parameter labels, info text */
#define UI_FONT_TITLE       &ui_font_roboto_24     /* Section titles */
#define UI_FONT_HEADER      &ui_font_roboto_28     /* Screen headers */
#define UI_FONT_VALUE       &ui_font_roboto_bold_48 /* Parameter values - 48px bold */
#define UI_FONT_LARGE       &ui_font_roboto_bold_48 /* Large values, numpad */

#ifdef __cplusplus
}
#endif
