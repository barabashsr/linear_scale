#pragma once

#include "esp_err.h"
#include <stdint.h>

esp_err_t buttons_init(void);
uint16_t buttons_read(void);
