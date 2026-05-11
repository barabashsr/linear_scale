#pragma once
#include "esp_err.h"
#include <stdbool.h>

esp_err_t keypad_init(void);
void     keypad_enable(bool enable);
bool     keypad_get_char(char *c);
