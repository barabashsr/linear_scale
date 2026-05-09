#pragma once

#include "config.h"

#include "esp_err.h"
#include <stdint.h>

typedef enum { SCALE_AXIAL = 0, SCALE_RADIAL = 1, SCALE_COUNT } scale_id_t;

esp_err_t  scale_init(scale_id_t id, int pin_a, int pin_b);
int32_t    scale_get_position(scale_id_t id);
void       scale_set_position(scale_id_t id, int32_t pos);
const char *scale_label(scale_id_t id);
