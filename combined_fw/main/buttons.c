#include "buttons.h"
#include "aw9523.h"
#include "i2c_protocol.h"
#include "esp_log.h"

static const char *TAG_BT = "buttons";

static const uint8_t aw_pins[] = {
    CFG_BTN_AW_RADIAL_T1, CFG_BTN_AW_RADIAL_T2,
    CFG_BTN_AW_AXIAL_T1,  CFG_BTN_AW_AXIAL_T2,
    CFG_BTN_AW_RD_TOGGLE, CFG_BTN_AW_ZERO,
};
static const uint16_t aw_masks[] = {
    BTN_RADIAL_T1, BTN_RADIAL_T2,
    BTN_AXIAL_T1,  BTN_AXIAL_T2,
    BTN_RD_TOGGLE, BTN_ZERO,
};
static const int aw_count = sizeof(aw_pins) / sizeof(aw_pins[0]);

esp_err_t buttons_init(void)
{
    ESP_LOGI(TAG_BT, "Buttons ready (%d via AW9523)", aw_count);
    return ESP_OK;
}

uint16_t buttons_read(void)
{
    uint16_t raw = 0;
    uint8_t aw_raw = aw9523_read_port0();

    for (int i = 0; i < aw_count; i++) {
        if ((aw_raw >> aw_pins[i]) & 1) raw |= aw_masks[i];
    }

    static uint16_t prev = 0, stable = 0;
    static int cnt = 0;
    if (raw == prev) {
        if (++cnt >= CFG_BTN_DEBOUNCE_TICKS) stable = raw;
    } else {
        cnt = 0;
    }
    prev = raw;
    return stable;
}
