#include "mcp23017.h"
#include "config.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG_MCP = "mcp23017";

esp_err_t mcp23017_write(uint8_t addr, uint8_t reg, uint8_t val)
{
    uint8_t data[2] = {reg, val};
    return i2c_master_write_to_device(CFG_I2C_BUS, addr, data, 2,
                                      pdMS_TO_TICKS(CFG_I2C_TOUT_SHORT));
}

esp_err_t mcp23017_read(uint8_t addr, uint8_t reg, uint8_t *val)
{
    return i2c_master_write_read_device(CFG_I2C_BUS, addr, &reg, 1, val, 1,
                                        pdMS_TO_TICKS(CFG_I2C_TOUT_SHORT));
}

esp_err_t mcp23017_init(uint8_t addr)
{
    uint8_t id;
    esp_err_t ret = mcp23017_read(addr, MCP_IODIRA, &id);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG_MCP, "MCP23017 not found at 0x%02X — keypad disabled", addr);
        return ret;
    }
    ESP_LOGI(TAG_MCP, "MCP23017 found at 0x%02X", addr);

    mcp23017_write(addr, 0x0A, 0x00);           /* IOCON: BANK=0 */
    mcp23017_write(addr, MCP_IODIRA, 0xF0);     /* GPA0-3=out, GPA4-7=in */
    mcp23017_write(addr, MCP_IODIRB, 0xFF);     /* GPIOB all inputs */
    mcp23017_write(addr, MCP_GPPUA,  0xF0);     /* pull-ups on GPA4-7 */
    mcp23017_write(addr, MCP_GPPUB,  0x00);     /* no pull-ups on GPIOB */
    mcp23017_write(addr, MCP_OLATA,  0x0F);     /* all rows HIGH (idle) */
    mcp23017_write(addr, MCP_OLATB,  0x00);

    ESP_LOGI(TAG_MCP, "MCP23017 ready (rows=A0-3 out, cols=A4-7 in pu)");
    return ESP_OK;
}
