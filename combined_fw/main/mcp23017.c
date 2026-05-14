#include "mcp23017.h"
#include "config.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG_MCP = "mcp23017";
static uint8_t g_olatb;

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
        ESP_LOGW(TAG_MCP, "MCP23017 not found at 0x%02X", addr);
        return ret;
    }
    ESP_LOGI(TAG_MCP, "MCP23017 found at 0x%02X", addr);

    mcp23017_write(addr, 0x0A, 0x00);
    mcp23017_write(addr, MCP_IODIRA, CFG_MCP_PORTA_DIR);
    mcp23017_write(addr, MCP_GPPUA,  CFG_MCP_PORTA_PU);
    mcp23017_write(addr, MCP_OLATA,  CFG_MCP_PORTA_OLAT);

    mcp23017_init_gpiob(addr);
    ESP_LOGI(TAG_MCP, "MCP23017 ready");
    return ESP_OK;
}

void mcp23017_init_gpiob(uint8_t addr)
{
    mcp23017_write(addr, MCP_IODIRB, CFG_MCP_PORTB_DIR);
    mcp23017_write(addr, MCP_GPPUB,  CFG_MCP_PORTB_PU);
    mcp23017_read(addr, MCP_OLATB, &g_olatb);
    ESP_LOGI(TAG_MCP, "MCP23017 GPIOB ready (LED=%d)", (g_olatb >> CFG_MCP_LED_RD) & 1);
}

void mcp23017_set_led(uint8_t addr, uint8_t pin, bool on)
{
    mcp23017_read(addr, MCP_OLATB, &g_olatb);
    if (on) g_olatb |=  (1 << pin);
    else    g_olatb &= ~(1 << pin);
    mcp23017_write(addr, MCP_OLATB, g_olatb);
    ESP_LOGI(TAG_MCP, "LED pin%d=%d OLATB=0x%02X", pin, on, g_olatb);
}
