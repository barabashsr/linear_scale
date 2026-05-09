#include "buttons.h"
#include "aw9523.h"
#include "i2c_protocol.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG_BT = "buttons";

static const int spare_gpios[CFG_BTN_SPARE_COUNT] = {
    CFG_BTN_RD_TOGGLE_GPIO, CFG_BTN_SPARE_GPIO
};
static const uint16_t spare_masks[CFG_BTN_SPARE_COUNT] = { BTN_RD_TOGGLE, 0 };

esp_err_t buttons_init(void)
{
    for (int i = 0; i < CFG_BTN_SPARE_COUNT; i++) {
        gpio_config_t io_conf = {
            .intr_type = GPIO_INTR_DISABLE,
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = (1ULL << spare_gpios[i]),
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
        };
        gpio_config(&io_conf);
    }
    ESP_LOGI(TAG_BT, "Buttons ready (8x AW9523 + %dx GPIO)", CFG_BTN_SPARE_COUNT);
    return ESP_OK;
}

uint16_t buttons_read(void)
{
    uint16_t raw = 0;
    uint8_t aw_raw = aw9523_read_port0();

    if (aw_raw & (1 << 0)) raw |= BTN_RADIAL_T1;
    if (aw_raw & (1 << 1)) raw |= BTN_RADIAL_T2;
    if (aw_raw & (1 << 2)) raw |= BTN_RADIAL_T3;
    if (aw_raw & (1 << 3)) raw |= BTN_RADIAL_T4;
    if (aw_raw & (1 << 4)) raw |= BTN_AXIAL_T1;
    if (aw_raw & (1 << 5)) raw |= BTN_AXIAL_T2;
    if (aw_raw & (1 << 6)) raw |= BTN_AXIAL_T3;
    if (aw_raw & (1 << 7)) raw |= BTN_AXIAL_T4;

    for (int i = 0; i < CFG_BTN_SPARE_COUNT; i++) {
        if (gpio_get_level(spare_gpios[i]) == 0) raw |= spare_masks[i];
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
