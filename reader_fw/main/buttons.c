#include "buttons.h"
#include "i2c_protocol.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "buttons";

static const int btn_gpios[BTN_COUNT] = {
    12, 13, 14, 15,
    16, 17, 18, 21,
    22, 23
};

static const uint16_t btn_masks[BTN_COUNT] = {
    BTN_RADIAL_T1, BTN_RADIAL_T2, BTN_RADIAL_T3, BTN_RADIAL_T4,
    BTN_AXIAL_T1,  BTN_AXIAL_T2,  BTN_AXIAL_T3,  BTN_AXIAL_T4,
    BTN_RD_TOGGLE, 0
};

static volatile uint16_t g_btn_stable = 0;
static esp_timer_handle_t g_btn_timer = NULL;

#define DEBOUNCE_TICKS 3

static void btn_timer_cb(void *arg)
{
    uint16_t raw = 0;
    for (int i = 0; i < BTN_COUNT; i++) {
        if (gpio_get_level(btn_gpios[i]) == 0) {
            raw |= btn_masks[i];
        }
    }

    static uint16_t prev_raw = 0;
    static uint16_t candidate = 0;
    static int stable_cnt = 0;

    if (raw == prev_raw) {
        if (++stable_cnt >= DEBOUNCE_TICKS) {
            candidate = raw;
        }
    } else {
        stable_cnt = 0;
    }
    prev_raw = raw;
    g_btn_stable = candidate;
}

esp_err_t buttons_init(void)
{
    uint64_t pin_mask = 0;
    for (int i = 0; i < BTN_COUNT; i++) {
        pin_mask |= (1ULL << btn_gpios[i]);
    }

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = pin_mask,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    const esp_timer_create_args_t timer_args = {
        .callback = btn_timer_cb,
        .name = "btn_poll",
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &g_btn_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(g_btn_timer, 10000));

    btn_timer_cb(NULL);
    ESP_LOGI(TAG, "Buttons initialized: %d GPIOs, 10ms polling, %d-tick debounce",
             BTN_COUNT, DEBOUNCE_TICKS);
    return ESP_OK;
}

uint16_t buttons_read(void)
{
    return g_btn_stable;
}
