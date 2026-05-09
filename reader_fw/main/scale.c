#include "scale.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "scale";

static pcnt_unit_handle_t g_pcnt_unit = NULL;
static volatile int32_t g_position = 0;

static bool IRAM_ATTR scale_on_reach(pcnt_unit_handle_t unit,
                                      const pcnt_watch_event_data_t *edata,
                                      void *user_ctx)
{
    BaseType_t high_task_wakeup = pdFALSE;
    int32_t *pos = (int32_t *)user_ctx;

    if (edata->watch_point_value == 20000) {
        *pos += 20000;
    } else {
        *pos -= 20000;
    }
    return (high_task_wakeup == pdTRUE);
}

esp_err_t scale_init(void)
{
    pcnt_unit_config_t unit_config = {
        .low_limit = -20000,
        .high_limit = 20000,
        .intr_priority = 0,
        .flags = {
            .accum_count = false,
        },
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &g_pcnt_unit));

    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 5000,
    };
    pcnt_unit_set_glitch_filter(g_pcnt_unit, &filter_config);

    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = SCALE_PIN_A,
        .level_gpio_num = SCALE_PIN_B,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(g_pcnt_unit, &chan_a_config, &pcnt_chan_a));

    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = SCALE_PIN_B,
        .level_gpio_num = SCALE_PIN_A,
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(g_pcnt_unit, &chan_b_config, &pcnt_chan_b));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b,
        PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    pcnt_unit_add_watch_point(g_pcnt_unit, 20000);
    pcnt_unit_add_watch_point(g_pcnt_unit, -20000);

    pcnt_event_callbacks_t cbs = {
        .on_reach = scale_on_reach,
    };
    pcnt_unit_register_event_callbacks(g_pcnt_unit, &cbs, (void *)&g_position);

    ESP_ERROR_CHECK(pcnt_unit_enable(g_pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(g_pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(g_pcnt_unit));

    ESP_LOGI(TAG, "PCNT initialized: A=GPIO%d B=GPIO%d", SCALE_PIN_A, SCALE_PIN_B);
    return ESP_OK;
}

int32_t scale_get_position(void)
{
    int count = 0;
    pcnt_unit_get_count(g_pcnt_unit, &count);
    return g_position + count;
}

void scale_set_position(int32_t pos)
{
    pcnt_unit_clear_count(g_pcnt_unit);
    g_position = pos;
}
