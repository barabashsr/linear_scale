#include "reader_comm.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "reader_comm";

esp_err_t reader_comm_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = READER_I2C_SDA,
        .scl_io_num = READER_I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = READER_I2C_FREQ,
    };
    ESP_ERROR_CHECK(i2c_param_config(READER_I2C_BUS, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(READER_I2C_BUS, I2C_MODE_MASTER, 0, 0, 0));

    ESP_LOGI(TAG, "I2C master ready: SCL=%d SDA=%d", READER_I2C_SCL, READER_I2C_SDA);
    return ESP_OK;
}

esp_err_t reader_comm_poll(i2c_packet_t *pkt)
{
    memset(pkt, 0, sizeof(*pkt));
    esp_err_t ret = i2c_master_read_from_device(
        READER_I2C_BUS, I2C_READER_ADDR,
        (uint8_t *)pkt, I2C_PACKET_SIZE,
        pdMS_TO_TICKS(50));

    if (ret != ESP_OK) {
        return ret;
    }

    uint8_t calc_crc = i2c_packet_crc((uint8_t *)pkt, sizeof(*pkt) - 1);
    if (calc_crc != pkt->crc) {
        return ESP_FAIL;
    }
    if (pkt->version != 0x01) {
        return ESP_FAIL;
    }

    return ESP_OK;
}
