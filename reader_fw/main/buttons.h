#pragma once

#include "esp_err.h"
#include <stdint.h>

#define BTN_DEBOUNCE_MS         50
#define BTN_COUNT               10

esp_err_t buttons_init(void);
uint16_t buttons_read(void);
