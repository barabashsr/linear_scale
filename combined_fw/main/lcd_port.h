#pragma once

#include "config.h"

#include "esp_err.h"

esp_err_t lcd_port_init(void);
esp_err_t lcd_port_bl_on(void);
esp_err_t lcd_port_bl_off(void);
