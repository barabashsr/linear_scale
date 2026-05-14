#pragma once
#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#define MCP23017_ADDR_BASE  0x20

/* registers (BANK=0) */
#define MCP_IODIRA   0x00
#define MCP_IODIRB   0x01
#define MCP_GPPUA    0x0C
#define MCP_GPPUB    0x0D
#define MCP_GPIOA    0x12
#define MCP_GPIOB    0x13
#define MCP_OLATA    0x14
#define MCP_OLATB    0x15

esp_err_t mcp23017_init(uint8_t addr);
void     mcp23017_init_gpiob(uint8_t addr);
esp_err_t mcp23017_write(uint8_t addr, uint8_t reg, uint8_t val);
esp_err_t mcp23017_read(uint8_t addr, uint8_t reg, uint8_t *val);
void     mcp23017_set_led(uint8_t addr, uint8_t pin, bool on);
