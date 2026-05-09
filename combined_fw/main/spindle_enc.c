#include "spindle_enc.h"
#include "driver/pulse_cnt.h"
#include "esp_attr.h"
#include "esp_log.h"

static const char *TAG_SE = "spindle";
static pcnt_unit_handle_t g_enc_unit = NULL;
static volatile int32_t g_enc_count = 0;

static bool IRAM_ATTR enc_on_reach(pcnt_unit_handle_t unit,
                                    const pcnt_watch_event_data_t *edata,
                                    void *user_ctx)
{
    int32_t *pos = (int32_t *)user_ctx;
    if (edata->watch_point_value > 0) *pos += CFG_PCNT_LIMIT;
    else                              *pos -= CFG_PCNT_LIMIT;
    return false;
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

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_a,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_a,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_b,
        PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_b,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    pcnt_unit_add_watch_point(g_enc_unit, CFG_PCNT_LIMIT);
    pcnt_unit_add_watch_point(g_enc_unit, -CFG_PCNT_LIMIT);

    pcnt_event_callbacks_t evt = { .on_reach = enc_on_reach };
    pcnt_unit_register_event_callbacks(g_enc_unit, &evt, (void *)&g_enc_count);

    ESP_ERROR_CHECK(pcnt_unit_enable(g_enc_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(g_enc_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(g_enc_unit));

    ESP_LOGI(TAG_SE, "Spindle encoder ready (A=%d B=%d)", CFG_SPINDLE_A, CFG_SPINDLE_B);
    return ESP_OK;
}

int32_t spindle_enc_get_count(void)
{
    int count = 0;
    pcnt_unit_get_count(g_enc_unit, &count);
    return g_enc_count + count;
}
