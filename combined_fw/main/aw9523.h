#include "config.h"

#include "esp_err.h"
#include <stdint.h>

esp_err_t aw9523_init(void);
esp_err_t aw9523_read_port0(uint8_t *val);
void aw9523_set_output(uint8_t pin, uint8_t level);
