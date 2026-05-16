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
#define CFG_LCD_HSYNC_BP        160
#define CFG_LCD_HSYNC_FP        20
#define CFG_LCD_VSYNC_PW        1
#define CFG_LCD_VSYNC_BP        35
#define CFG_LCD_VSYNC_FP        5
#define CFG_LCD_PCLK_NEG        0
#define CFG_LCD_DATA_WIDTH      16
#define CFG_LCD_BPP             16
#define CFG_LCD_BOUNCE_H        30
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
#define CFG_AW9523_ADDR         0x5B
#define CFG_AW9523_PIN_LCD_RST  10
#define CFG_AW9523_PIN_TP_RST   11
#define CFG_AW9523_RST_LOW_MS   10
#define CFG_AW9523_RST_DELAY_MS 50

/* ── AW9523 button inputs (port0) ────────────── */
#define CFG_BTN_AW_DIAMETER     1
#define CFG_BTN_AW_RADIAL_T1    0
#define CFG_BTN_AW_RD_TOGGLE    3
#define CFG_BTN_AW_RADIAL_T2    2
#define CFG_BTN_AW_AXIAL_ZERO   5
#define CFG_BTN_AW_AXIAL_T1     4

#define CFG_BTN_AW_AXIAL_T2     6
#define CFG_BTN_AW_DIAM_ZERO    7





/* ── MCP23017 keypad ─────────────────────────── */
#define CFG_MCP23017_ADDR       0x20
#define CFG_KP_POLL_MS          10
#define CFG_KP_DEBOUNCE         3

#define CFG_MCP_PORTA_DIR       0xF0
#define CFG_MCP_PORTA_PU        0xF0
#define CFG_MCP_PORTA_OLAT      0x0F

#define CFG_MCP_PORTB_DIR       0x03
#define CFG_MCP_PORTB_PU        0x03
#define CFG_MCP_PORTB_OLAT      0x00
#define CFG_MCP_LED_RD          2
#define CFG_MCP_BTN_DIAM_T1     0
#define CFG_MCP_BTN_DIAM_T2     1



/* ── Linear scales (PCNT quadrature) ─────────── */
#define CFG_SCALE_AXIAL_A       GPIO_NUM_19
#define CFG_SCALE_AXIAL_B       GPIO_NUM_40
#define CFG_SCALE_RADIAL_A      GPIO_NUM_42
#define CFG_SCALE_RADIAL_B      GPIO_NUM_2
#define CFG_SCALE_MM_PER_COUNT  0.005f
#define CFG_SCALE_AXIAL_MM      0.005f
#define CFG_SCALE_RADIAL_MM     0.005f
#define CFG_PCNT_LIMIT          20000
#define CFG_PCNT_GLITCH_NS      5000

/* ── Spindle encoder ─────────────────────────── */
#define CFG_SPINDLE_A           GPIO_NUM_20
#define CFG_SPINDLE_B           GPIO_NUM_41
#define CFG_SPINDLE_Z           GPIO_NUM_4
#define CFG_SPINDLE_CPR         400.0f
#define CFG_SPINDLE_GEAR_NUM    1
#define CFG_SPINDLE_GEAR_DEN    1
#define CFG_SPINDLE_RPM_MS      500

/* ── Physical buttons (debounce) ─────────────── */
#define CFG_BTN_POLL_MS          20
#define CFG_BTN_DEBOUNCE_TICKS  8

/* ── Screen position offset ──────────────────── */
#define CFG_UI_X_OFFSET          20

/* ── UI layout ───────────────────────────────── */
#define CFG_UI_HALF_W           380
#define CFG_UI_PAD              10
#define CFG_UI_MARGIN           10
#define CFG_UI_SP_V_GAP         8

/* ── UI colors ───────────────────────────────── */
#define CFG_COLOR_BG            0x1A1A2E
#define CFG_COLOR_PANEL_BG      0x16213E
#define CFG_COLOR_ACCENT        0x0F3460
#define CFG_COLOR_HIGHLIGHT     0xE94560
#define CFG_COLOR_TEXT          0xEEEEEE
#define CFG_COLOR_TEXT_DIM      0x888888
#define CFG_COLOR_GREEN         0x2ECC71
#define CFG_COLOR_SCARLET      0xFF2400
#define CFG_COLOR_BTN_BG        0x1A3A5C
#define CFG_COLOR_CONFIRM_BG    0xC0392B
#define CFG_COLOR_CANCEL_BG     0x7F8C8D

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
#define CFG_LV_SCARLET          lv_color_hex(CFG_COLOR_SCARLET)

/* ── UI fonts ────────────────────────────────── */
#define CFG_FONT_CONN           &lv_font_montserrat_14
#define CFG_FONT_SMALL          UI_FONT_SMALL
#define CFG_FONT_BTN            UI_FONT_LABEL
#define CFG_FONT_SP_VALUE       UI_FONT_SETPOINT
#define CFG_FONT_MAIN_VALUE     UI_FONT_MAIN
#define CFG_FONT_MAIN_LABEL     UI_FONT_LABEL
#define CFG_FONT_TITLE          UI_FONT_TITLE

/* ── UI styles ───────────────────────────────── */
#define CFG_UI_PANEL_BORDER     1
#define CFG_UI_PANEL_RADIUS     8
#define CFG_UI_PANEL_PAD        8
#define CFG_UI_BTN_RADIUS       6
#define CFG_UI_BTN_PAD          6
#define CFG_UI_DANGER_RADIUS    6
#define CFG_UI_DANGER_PAD       6

/* ── Setpoints ───────────────────────────────── */
#define CFG_MAX_SETPOINTS       2

/* ── Numpad modal ────────────────────────────── */
#define CFG_NUMPAD_COLS         4
#define CFG_NUMPAD_BTN_W        60
#define CFG_NUMPAD_BTN_H        50

/* ── Diameter modal ──────────────────────────── */
#define CFG_DIAM_MIN_MM         0.0f
#define CFG_DIAM_MAX_MM         500.0f
#define CFG_DIAM_SLIDER_RES     5000
#define CFG_DIAM_DEFAULT_MM      25.0f

/* ── Application ─────────────────────────────── */
#define CFG_APP_POLL_MS         1
#define CFG_APP_POS_BUF_LEN     32

/* ── I2C timeouts (ms) ───────────────────────── */
#define CFG_I2C_TOUT_SHORT      10
#define CFG_I2C_TOUT_NORMAL     100
