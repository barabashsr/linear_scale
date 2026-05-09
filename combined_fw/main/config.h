#pragma once

#include "driver/gpio.h"

/* ── I2C bus ─────────────────────────────────── */
#define CFG_I2C_BUS             I2C_NUM_0
#define CFG_I2C_SCL             47
#define CFG_I2C_SDA             48
#define CFG_I2C_FREQ_HZ         400000

/* ── LCD RGB panel ───────────────────────────── */
#define CFG_LCD_H_RES           800
#define CFG_LCD_V_RES           480
#define CFG_LCD_PCLK_MHZ        24
#define CFG_LCD_PCLK_HZ         (CFG_LCD_PCLK_MHZ * 1000 * 1000)
#define CFG_LCD_HSYNC_PW        1
#define CFG_LCD_HSYNC_BP        87
#define CFG_LCD_HSYNC_FP        20
#define CFG_LCD_VSYNC_PW        1
#define CFG_LCD_VSYNC_BP        31
#define CFG_LCD_VSYNC_FP        5
#define CFG_LCD_PCLK_NEG        0
#define CFG_LCD_DATA_WIDTH      16
#define CFG_LCD_BPP             16
#define CFG_LCD_BOUNCE_H        10
#define CFG_LCD_FB_COUNT        2
#define CFG_LCD_FB_IN_PSRAM     1
#define CFG_LCD_SRAM_ALIGN      4
#define CFG_LCD_PSRAM_ALIGN     64

#define CFG_LCD_D0              17
#define CFG_LCD_D1              16
#define CFG_LCD_D2              15
#define CFG_LCD_D3              7
#define CFG_LCD_D4              6
#define CFG_LCD_D5              21
#define CFG_LCD_D6              0
#define CFG_LCD_D7              46
#define CFG_LCD_D8              3
#define CFG_LCD_D9              8
#define CFG_LCD_D10             18
#define CFG_LCD_D11             10
#define CFG_LCD_D12             11
#define CFG_LCD_D13             12
#define CFG_LCD_D14             13
#define CFG_LCD_D15             14
#define CFG_LCD_PCLK            9
#define CFG_LCD_HSYNC           5
#define CFG_LCD_VSYNC           38
#define CFG_LCD_DE              39
#define CFG_LCD_DISP            (-1)

/* ── Backlight ───────────────────────────────── */
#define CFG_BL_GPIO             GPIO_NUM_45
#define CFG_BL_FREQ_HZ          5000
#define CFG_BL_DUTY_MAX         1023
#define CFG_BL_DUTY_OFF         0

/* ── AW9523B IO expander ─────────────────────── */
#define CFG_AW9523_ADDR         0x5D
#define CFG_AW9523_PIN_LCD_RST  10
#define CFG_AW9523_PIN_TP_RST   11
#define CFG_AW9523_RST_LOW_MS   10
#define CFG_AW9523_RST_DELAY_MS 50

/* ── Linear scales (PCNT quadrature) ─────────── */
#define CFG_SCALE_AXIAL_A       GPIO_NUM_20
#define CFG_SCALE_AXIAL_B       GPIO_NUM_19
#define CFG_SCALE_RADIAL_A      GPIO_NUM_41
#define CFG_SCALE_RADIAL_B      GPIO_NUM_40

#define CFG_SCALE_MM_PER_COUNT  0.005f
#define CFG_PCNT_LIMIT          20000
#define CFG_PCNT_GLITCH_NS      5000

/* ── Spindle encoder ─────────────────────────── */
#define CFG_SPINDLE_A           GPIO_NUM_4
#define CFG_SPINDLE_B           GPIO_NUM_1
#define CFG_SPINDLE_CPR         400.0f

/* ── Physical buttons ────────────────────────── */
#define CFG_BTN_RD_TOGGLE_GPIO  GPIO_NUM_42
#define CFG_BTN_SPARE_GPIO      GPIO_NUM_2
#define CFG_BTN_DEBOUNCE_TICKS  3
#define CFG_BTN_POLL_US         10000
#define CFG_BTN_SPARE_COUNT     2

/* ── UI layout ───────────────────────────────── */
#define CFG_UI_HALF_W           390
#define CFG_UI_FULL_H           480
#define CFG_UI_PAD              8
#define CFG_UI_AXIAL_XOFF       6
#define CFG_UI_MARGIN           16

/* ── UI widget sizes ─────────────────────────── */
#define CFG_UI_CONN_H           20
#define CFG_UI_GAP_TITLE        22
#define CFG_UI_TITLE_H          24
#define CFG_UI_GAP_VALUE        28
#define CFG_UI_MAIN_VALUE_H     52
#define CFG_UI_MAIN_LABEL_H     20
#define CFG_UI_GAP_LABEL        24
#define CFG_UI_BTN_ZERO_H       36
#define CFG_UI_GAP_SP           44
#define CFG_UI_SP_ROW_H         44
#define CFG_UI_SP_BTN_SIZE      32
#define CFG_UI_SP_ROW_GAP       48
#define CFG_UI_ENTRY_H          40
#define CFG_UI_ENTRY_TA_W       80
#define CFG_UI_ENTRY_TA_H       28
#define CFG_UI_ENTRY_TA_MAXLEN  8
#define CFG_UI_ENTRY_OK_W       36
#define CFG_UI_ENTRY_OK_H       28
#define CFG_UI_SPINDLE_H        36
#define CFG_UI_SPINDLE_BTN_W    32
#define CFG_UI_SPINDLE_BTN_H    28
#define CFG_UI_SPINDLE_YOFF     44

/* ── UI dialog ───────────────────────────────── */
#define CFG_UI_DLG_W            280
#define CFG_UI_DLG_H            120
#define CFG_UI_DLG_BORDER       2
#define CFG_UI_DLG_RADIUS       8
#define CFG_UI_DLG_TITLE_Y      12
#define CFG_UI_DLG_BTN_W        100
#define CFG_UI_DLG_BTN_H        36
#define CFG_UI_DLG_BTN_XOFF     20
#define CFG_UI_DLG_BTN_YOFF     (-12)

/* ── UI styles ───────────────────────────────── */
#define CFG_UI_PANEL_BORDER     1
#define CFG_UI_PANEL_RADIUS     8
#define CFG_UI_PANEL_PAD        6
#define CFG_UI_BTN_RADIUS       4
#define CFG_UI_BTN_PAD          4
#define CFG_UI_DANGER_RADIUS    4
#define CFG_UI_DANGER_PAD       4

/* ── UI colors ───────────────────────────────── */
#define CFG_COLOR_BG            0x1A1A2E
#define CFG_COLOR_PANEL_BG      0x16213E
#define CFG_COLOR_ACCENT        0x0F3460
#define CFG_COLOR_HIGHLIGHT     0xE94560
#define CFG_COLOR_TEXT          0xEEEEEE
#define CFG_COLOR_TEXT_DIM      0x888888
#define CFG_COLOR_GREEN         0x2ECC71
#define CFG_COLOR_BTN_BG        0x1A3A5C
#define CFG_COLOR_CONFIRM_BG    0xC0392B
#define CFG_COLOR_CANCEL_BG     0x7F8C8D

/* ── UI colors as lv_color_t ─────────────────── */
#define CFG_LV_BG               lv_color_hex(CFG_COLOR_BG)
#define CFG_LV_PANEL_BG         lv_color_hex(CFG_COLOR_PANEL_BG)
#define CFG_LV_ACCENT           lv_color_hex(CFG_COLOR_ACCENT)
#define CFG_LV_HIGHLIGHT        lv_color_hex(CFG_COLOR_HIGHLIGHT)
#define CFG_LV_TEXT             lv_color_hex(CFG_COLOR_TEXT)
#define CFG_LV_TEXT_DIM         lv_color_hex(CFG_COLOR_TEXT_DIM)
#define CFG_LV_GREEN            lv_color_hex(CFG_COLOR_GREEN)
#define CFG_LV_BTN_BG           lv_color_hex(CFG_COLOR_BTN_BG)
#define CFG_LV_CONFIRM_BG       lv_color_hex(CFG_COLOR_CONFIRM_BG)
#define CFG_LV_CANCEL_BG        lv_color_hex(CFG_COLOR_CANCEL_BG)

/* ── UI fonts ────────────────────────────────── */
#define CFG_FONT_CONN           &lv_font_montserrat_14
#define CFG_FONT_SMALL          &lv_font_montserrat_12
#define CFG_FONT_BTN            &lv_font_montserrat_16
#define CFG_FONT_SP_VALUE       &lv_font_montserrat_20
#define CFG_FONT_SPINDLE        &lv_font_montserrat_20
#define CFG_FONT_MAIN_VALUE     &lv_font_montserrat_48
#define CFG_FONT_MAIN_LABEL     &lv_font_montserrat_24
#define CFG_FONT_TITLE          &lv_font_montserrat_20

/* ── Application ─────────────────────────────── */
#define CFG_APP_POLL_MS         25
#define CFG_APP_LVGL_LOCK_MS    50
#define CFG_APP_INIT_LOCK_MS    5000
#define CFG_APP_TASK_STACK      4096
#define CFG_APP_TASK_PRIO       5
#define CFG_APP_POS_BUF_LEN     32
#define CFG_APP_ZERO_THRESHOLD  0.0005f

/* ── Number of setpoints per axis ────────────── */
#define CFG_MAX_SETPOINTS       4

/* ── I2C timeouts (ms) ───────────────────────── */
#define CFG_I2C_TOUT_SHORT      10
#define CFG_I2C_TOUT_NORMAL     100
