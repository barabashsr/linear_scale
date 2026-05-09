#pragma once

#include "driver/pulse_cnt.h"
#include "esp_err.h"

#define SCALE_PCNT_UNIT         0
#define SCALE_PIN_A             4
#define SCALE_PIN_B             5

esp_err_t scale_init(void);
int32_t scale_get_position(void);
void scale_set_position(int32_t pos);
