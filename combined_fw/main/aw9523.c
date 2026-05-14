#include "aw9523.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG_AW = "aw9523";

enum { REG_IN_PORT0 = 0x00, REG_OUT_PORT0 = 0x02, REG_CFG_PORT0 = 0x04,
       REG_OUT_PORT1 = 0x03, REG_CFG_PORT1 = 0x05, REG_ID = 0x10 };

esp_err_t aw9523_init(void)
{
    uint8_t data[2];
    esp_err_t ret;

    data[0] = REG_ID;
    ret = i2c_master_write_read_device(CFG_I2C_BUS, CFG_AW9523_ADDR,
        data, 1, data, 1, pdMS_TO_TICKS(CFG_I2C_TOUT_NORMAL));
    if (ret != ESP_OK) {
        ESP_LOGW(TAG_AW, "AW9523 not found at 0x%02X (ret=0x%x)", CFG_AW9523_ADDR, ret);
        return ret;
    }
    ESP_LOGI(TAG_AW, "AW9523 found at 0x%02X, ID=0x%02X", CFG_AW9523_ADDR, data[0]);

    data[0] = 0x12; data[1] = 0xFF;              /* MODE0: all pins GPIO mode (not LED) */
    ret = i2c_master_write_to_device(CFG_I2C_BUS, CFG_AW9523_ADDR,
        data, 2, pdMS_TO_TICKS(CFG_I2C_TOUT_NORMAL));
    if (ret != ESP_OK) return ret;

    data[0] = 0x11; data[1] = (1 << 4);          /* GCR: open-drain (bit4) → pull-ups via OUT reg */
    ret = i2c_master_write_to_device(CFG_I2C_BUS, CFG_AW9523_ADDR,
        data, 2, pdMS_TO_TICKS(CFG_I2C_TOUT_NORMAL));
    if (ret != ESP_OK) return ret;

    data[0] = REG_CFG_PORT0; data[1] = 0xFF;
    ret = i2c_master_write_to_device(CFG_I2C_BUS, CFG_AW9523_ADDR,
        data, 2, pdMS_TO_TICKS(CFG_I2C_TOUT_NORMAL));
    if (ret != ESP_OK) return ret;

    data[0] = REG_OUT_PORT0; data[1] = 0xFF;  /* pull-ups on port0 inputs */
    ret = i2c_master_write_to_device(CFG_I2C_BUS, CFG_AW9523_ADDR,
        data, 2, pdMS_TO_TICKS(CFG_I2C_TOUT_NORMAL));
    if (ret != ESP_OK) return ret;

    data[0] = REG_CFG_PORT1; data[1] = 0xFF;
    ret = i2c_master_write_to_device(CFG_I2C_BUS, CFG_AW9523_ADDR,
        data, 2, pdMS_TO_TICKS(CFG_I2C_TOUT_NORMAL));
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG_AW, "AW9523 ready (all inputs)");
    return ESP_OK;
}

uint8_t aw9523_read_port0(void)
{
    uint8_t reg = REG_IN_PORT0, val = 0;
    i2c_master_write_read_device(CFG_I2C_BUS, CFG_AW9523_ADDR,
        &reg, 1, &val, 1, pdMS_TO_TICKS(CFG_I2C_TOUT_SHORT));
    return val;
}

void aw9523_set_output(uint8_t pin, uint8_t level)
{
    uint8_t port = (pin < 8) ? 0 : 1;
    uint8_t bit  = (pin < 8) ? pin : (pin - 8);
    uint8_t reg  = (port == 0) ? REG_OUT_PORT0 : REG_OUT_PORT1;

    uint8_t cur;
    i2c_master_write_read_device(CFG_I2C_BUS, CFG_AW9523_ADDR,
        &reg, 1, &cur, 1, pdMS_TO_TICKS(CFG_I2C_TOUT_SHORT));
    if (level) cur |=  (1 << bit);
    else       cur &= ~(1 << bit);

    uint8_t data[2] = {reg, cur};
    i2c_master_write_to_device(CFG_I2C_BUS, CFG_AW9523_ADDR,
        data, 2, pdMS_TO_TICKS(CFG_I2C_TOUT_SHORT));
}
