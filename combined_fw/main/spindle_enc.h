#pragma once

#include "config.h"

#include "esp_err.h"
#include <stdint.h>

esp_err_t spindle_enc_init(void);
int32_t spindle_enc_get_count(void);
