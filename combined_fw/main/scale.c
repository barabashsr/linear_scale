#include "scale.h"
#include "driver/pulse_cnt.h"
#include "esp_attr.h"
#include "esp_log.h"

static const char *TAG_SC = "scale";

typedef struct {
    pcnt_unit_handle_t unit;
    volatile int32_t   position;
} scale_ctx_t;

static scale_ctx_t g_ctx[SCALE_COUNT];

static bool IRAM_ATTR on_reach(pcnt_unit_handle_t unit,
                                const pcnt_watch_event_data_t *edata,
                                void *user_ctx)
{
    int32_t *pos = (int32_t *)user_ctx;
    if (edata->watch_point_value > 0) *pos += CFG_PCNT_LIMIT;
    else                              *pos -= CFG_PCNT_LIMIT;
    return false;
}

esp_err_t scale_init(scale_id_t id, int pin_a, int pin_b)
{
    scale_ctx_t *ctx = &g_ctx[id];

    pcnt_unit_config_t cfg = {
        .low_limit = -CFG_PCNT_LIMIT,
        .high_limit = CFG_PCNT_LIMIT,
        .flags = { .accum_count = false },
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&cfg, &ctx->unit));

    pcnt_glitch_filter_config_t flt = { .max_glitch_ns = CFG_PCNT_GLITCH_NS };
    pcnt_unit_set_glitch_filter(ctx->unit, &flt);

    pcnt_chan_config_t ch_a = { .edge_gpio_num = pin_a, .level_gpio_num = pin_b };
    pcnt_channel_handle_t chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(ctx->unit, &ch_a, &chan_a));

    pcnt_chan_config_t ch_b = { .edge_gpio_num = pin_b, .level_gpio_num = pin_a };
    pcnt_channel_handle_t chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(ctx->unit, &ch_b, &chan_b));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_a,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_a,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_b,
        PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_b,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    pcnt_unit_add_watch_point(ctx->unit, CFG_PCNT_LIMIT);
    pcnt_unit_add_watch_point(ctx->unit, -CFG_PCNT_LIMIT);

    pcnt_event_callbacks_t evt = { .on_reach = on_reach };
    pcnt_unit_register_event_callbacks(ctx->unit, &evt, (void *)&ctx->position);

    ESP_ERROR_CHECK(pcnt_unit_enable(ctx->unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(ctx->unit));
    ESP_ERROR_CHECK(pcnt_unit_start(ctx->unit));

    ESP_LOGI(TAG_SC, "%s scale ready (A=%d B=%d)", scale_label(id), pin_a, pin_b);
    return ESP_OK;
}

int32_t scale_get_position(scale_id_t id)
{
    int count = 0;
    pcnt_unit_get_count(g_ctx[id].unit, &count);
    return g_ctx[id].position + count;
}

void scale_set_position(scale_id_t id, int32_t pos)
{
    pcnt_unit_clear_count(g_ctx[id].unit);
    g_ctx[id].position = pos;
}

const char *scale_label(scale_id_t id)
{
    return (id == SCALE_AXIAL) ? "Axial" : "Radial";
}
