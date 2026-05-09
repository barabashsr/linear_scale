#include "scale.h"
#include "buttons.h"
#include "i2c_slave.h"
#include "i2c_protocol.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

static const char *TAG = "reader";

static void reader_task(void *arg)
{
    while (1) {
        i2c_packet_t pkt;
        pkt.version = 0x01;
        pkt.btn_state = buttons_read();
        pkt.position_005mm = scale_get_position();

        uint8_t *raw = (uint8_t *)&pkt;
        pkt.crc = i2c_packet_crc(raw, sizeof(pkt) - 1);

        i2c_reset_tx_fifo(I2C_SLAVE_NUM);
        i2c_slave_write_buffer(I2C_SLAVE_NUM, raw, sizeof(pkt), portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Scale reader firmware starting");

    ESP_ERROR_CHECK(scale_init());
    ESP_ERROR_CHECK(buttons_init());
    i2c_slave_init();

    xTaskCreate(reader_task, "reader", 4096, NULL, 5, NULL);
}
