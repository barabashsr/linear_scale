#include "config.h"

#include "esp_err.h"
#include <stdint.h>

esp_err_t aw9523_init(void);
uint8_t aw9523_read_port0(void);
void aw9523_set_output(uint8_t pin, uint8_t level);
