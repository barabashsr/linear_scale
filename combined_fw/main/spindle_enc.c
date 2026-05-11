#include "spindle_enc.h"
#include "driver/pulse_cnt.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG_SE = "spindle";
static pcnt_unit_handle_t g_enc_unit = NULL;
static volatile int32_t g_enc_count = 0;
static volatile int32_t g_z_count = 0;
static int64_t g_last_us = 0;
static float g_rpm = 0;

static bool IRAM_ATTR enc_on_reach(pcnt_unit_handle_t unit,
                                    const pcnt_watch_event_data_t *edata,
                                    void *user_ctx)
{
    int32_t *pos = (int32_t *)user_ctx;
    if (edata->watch_point_value > 0) *pos += CFG_PCNT_LIMIT;
    else                              *pos -= CFG_PCNT_LIMIT;
    pcnt_unit_clear_count(unit);
    return false;
}

static void IRAM_ATTR z_isr_handler(void *arg)
{
    g_z_count++;
}

esp_err_t spindle_enc_init(void)
{
    pcnt_unit_config_t cfg = {
        .low_limit = -CFG_PCNT_LIMIT,
        .high_limit = CFG_PCNT_LIMIT,
        .flags = { .accum_count = false },
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&cfg, &g_enc_unit));

    pcnt_glitch_filter_config_t flt = { .max_glitch_ns = CFG_PCNT_GLITCH_NS };
    pcnt_unit_set_glitch_filter(g_enc_unit, &flt);

    pcnt_chan_config_t ch_a = { .edge_gpio_num = CFG_SPINDLE_A, .level_gpio_num = CFG_SPINDLE_B };
    pcnt_channel_handle_t chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(g_enc_unit, &ch_a, &chan_a));
    pcnt_chan_config_t ch_b = { .edge_gpio_num = CFG_SPINDLE_B, .level_gpio_num = CFG_SPINDLE_A };
    pcnt_channel_handle_t chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(g_enc_unit, &ch_b, &chan_b));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_b, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    pcnt_unit_add_watch_point(g_enc_unit, CFG_PCNT_LIMIT);
    pcnt_unit_add_watch_point(g_enc_unit, -CFG_PCNT_LIMIT);
    pcnt_event_callbacks_t evt = { .on_reach = enc_on_reach };
    pcnt_unit_register_event_callbacks(g_enc_unit, &evt, (void *)&g_enc_count);

    ESP_ERROR_CHECK(pcnt_unit_enable(g_enc_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(g_enc_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(g_enc_unit));

    // Z-signal (index pulse) on rising edge
    gpio_config_t z_conf = {
        .intr_type = GPIO_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << CFG_SPINDLE_Z),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&z_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(CFG_SPINDLE_Z, z_isr_handler, NULL);

    g_last_us = esp_timer_get_time();
    ESP_LOGI(TAG_SE, "Spindle ready A=%d B=%d Z=%d CPR=%.0f gear=%d:%d",
             CFG_SPINDLE_A, CFG_SPINDLE_B, CFG_SPINDLE_Z,
             CFG_SPINDLE_CPR, CFG_SPINDLE_GEAR_NUM, CFG_SPINDLE_GEAR_DEN);
    return ESP_OK;
}

int32_t spindle_enc_get_count(void)
{
    int count = 0;
    pcnt_unit_get_count(g_enc_unit, &count);
    return g_enc_count + count;
}

float spindle_get_rpm(void) { return g_rpm; }

void spindle_zero_z(void) { g_z_count = 0; }

void spindle_update_rpm(void)
{
    int64_t now = esp_timer_get_time();
    int64_t dt = now - g_last_us;
    g_last_us = now;
    if (dt <= 0) return;

    int32_t raw = g_enc_count;
    int count = 0;
    pcnt_unit_get_count(g_enc_unit, &count);
    raw += count;

    static int32_t prev_raw = 0;
    float ratio = (float)CFG_SPINDLE_GEAR_NUM / (float)CFG_SPINDLE_GEAR_DEN;
    float revs = (raw - prev_raw) * ratio / CFG_SPINDLE_CPR;
    g_rpm = revs * (60000000.0f / dt);
    prev_raw = raw;
}
