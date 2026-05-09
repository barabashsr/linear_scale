#include "i2c_slave.h"
#include "i2c_protocol.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "i2c_slave";

esp_err_t i2c_slave_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_SLAVE,
        .sda_io_num = I2C_SLAVE_SDA_IO,
        .scl_io_num = I2C_SLAVE_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .slave = {
            .slave_addr = I2C_READER_ADDR,
        },
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_SLAVE_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_SLAVE_NUM, I2C_MODE_SLAVE,
                                       I2C_SLAVE_RX_BUF_LEN,
                                       I2C_SLAVE_TX_BUF_LEN, 0));

    ESP_LOGI(TAG, "I2C slave initialized at 0x%02X (SDA=%d SCL=%d)",
             I2C_READER_ADDR, I2C_SLAVE_SDA_IO, I2C_SLAVE_SCL_IO);
    return ESP_OK;
}


