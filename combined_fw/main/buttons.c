#include "buttons.h"
#include "aw9523.h"
#include "mcp23017.h"
#include "i2c_protocol.h"
#include "esp_log.h"

static const char *TAG_BT = "buttons";

static const uint8_t aw_pins[] = {
    CFG_BTN_AW_RADIAL_T1,   CFG_BTN_AW_DIAMETER,
    CFG_BTN_AW_RADIAL_T2,   CFG_BTN_AW_RD_TOGGLE,
    CFG_BTN_AW_AXIAL_T1,    CFG_BTN_AW_AXIAL_ZERO,
    CFG_BTN_AW_AXIAL_T2,    CFG_BTN_AW_DIAM_ZERO,
};
static const uint16_t aw_masks[] = {
    BTN_RADIAL_T1,  BTN_DIAMETER,
    BTN_RADIAL_T2,  BTN_RD_TOGGLE,
    BTN_AXIAL_T1,   BTN_AXIAL_ZERO,
    BTN_AXIAL_T2,   BTN_DIAM_ZERO,
};
static const int aw_count = sizeof(aw_pins) / sizeof(aw_pins[0]);

esp_err_t buttons_init(void)
{
    esp_err_t ret = ESP_OK;

    uint8_t dummy;
    if (aw9523_read_port0(&dummy) == ESP_OK) {
        ESP_LOGI(TAG_BT, "AW9523 port0 ok (0x%02X)", dummy);
    } else {
        ESP_LOGE(TAG_BT, "AW9523 port0 NOT accessible");
        ret = ESP_FAIL;
    }

    if (mcp23017_read(CFG_MCP23017_ADDR, MCP_GPIOB, &dummy) == ESP_OK) {
        ESP_LOGI(TAG_BT, "MCP23017 GPIOB ok (0x%02X)", dummy);
    } else {
        ESP_LOGE(TAG_BT, "MCP23017 GPIOB NOT accessible");
        ret = ESP_FAIL;
    }

    ESP_LOGI(TAG_BT, "Buttons ready (%d AW9523 + 2 MCP23017)", aw_count);
    return ret;
}

uint16_t buttons_read(void)
{
    uint16_t raw = 0;

    /* ── AW9523 port0 ── */
    uint8_t aw_raw = 0xFF;
    uint16_t aw_bits = 0;
    aw9523_read_port0(&aw_raw);
    for (int i = 0; i < aw_count; i++) {
        if (!((aw_raw >> aw_pins[i]) & 1)) aw_bits |= aw_masks[i];
    }

    /* ── MCP23017 GPIOB ── */
    uint8_t gb = 0xFF;
    uint16_t mcp_bits = 0;
    if (mcp23017_read(CFG_MCP23017_ADDR, MCP_GPIOB, &gb) == ESP_OK) {
        if (!((gb >> CFG_MCP_BTN_DIAM_T1) & 1)) mcp_bits |= BTN_DIAM_T1;
        if (!((gb >> CFG_MCP_BTN_DIAM_T2) & 1)) mcp_bits |= BTN_DIAM_T2;
    }

    /* ── separate debounce: AW + MCP ── */
    static uint16_t aw_prev, mcp_prev, aw_stable, mcp_stable;
    static int aw_cnt, mcp_cnt;

    if (aw_bits == aw_prev) aw_cnt++; else aw_cnt = 0;
    aw_prev = aw_bits;
    if (aw_cnt >= CFG_BTN_DEBOUNCE_TICKS) aw_stable = aw_bits;

    if (mcp_bits == mcp_prev) mcp_cnt++; else mcp_cnt = 0;
    mcp_prev = mcp_bits;
    if (mcp_cnt >= CFG_BTN_DEBOUNCE_TICKS) mcp_stable = mcp_bits;

    raw = aw_stable | mcp_stable;

    /* ── logging: every ~500ms + on raw changes ── */
    static int call_cnt;
    call_cnt++;
    bool periodic = (call_cnt % 500 == 0);
    static uint16_t last_log;
    bool changed = (raw != last_log);

    if (periodic || changed) {
        ESP_LOGI(TAG_BT, "#%d AW=0x%02X GB=0x%02X aw_b=0x%04X mcp_b=0x%04X "
                 "aw_c=%d mcp_c=%d st=0x%04X",
                 call_cnt, aw_raw, gb, aw_bits, mcp_bits, aw_cnt, mcp_cnt, raw);
        if (changed) last_log = raw;
    }

    return raw;
}
