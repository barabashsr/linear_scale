#include "waveshare_rgb_lcd_port.h"
#include "lvgl_port.h"
#include "reader_comm.h"
#include "ui_logic.h"
#include "ui_styles.h"
#include "ui_main.h"
#include "i2c_protocol.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG_MAIN = "main";

static void poller_task(void *arg)
{
    i2c_packet_t pkt;
    int fail_count = 0;

    while (1) {
        esp_err_t ret = reader_comm_poll(&pkt);

        if (lvgl_port_lock(50)) {
            app_state_t *st = ui_logic_get_state();
            if (ret == ESP_OK) {
                st->reader_ok = true;
                fail_count = 0;
                ui_logic_update_position(pkt.position_005mm);
                ui_logic_handle_button(pkt.btn_state);
            } else {
                fail_count++;
                if (fail_count > 10) {
                    st->reader_ok = false;
                }
            }
            ui_main_update();
            lvgl_port_unlock();
        }

        vTaskDelay(pdMS_TO_TICKS(READER_POLL_MS));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG_MAIN, "Linear Scale Display firmware starting");

    ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());
    ESP_ERROR_CHECK(wavesahre_rgb_lcd_bl_on());

    ESP_ERROR_CHECK(reader_comm_init());

    ui_logic_init();

    if (lvgl_port_lock(5000)) {
        ui_styles_init();
        ui_main_create();
        lvgl_port_unlock();
    }

    xTaskCreate(poller_task, "poller", 4096, NULL, 5, NULL);
}
