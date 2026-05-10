#pragma once

#include "config.h"

#include "esp_err.h"

esp_err_t lcd_port_init(void);
esp_err_t lcd_port_bl_on(void);
esp_err_t lcd_port_bl_off(void);
void *lcd_port_get_panel_handle(void);
void *lcd_port_get_fb0(void);
void *lcd_port_get_fb1(void);
void *lcd_port_get_touch_handle(void);
