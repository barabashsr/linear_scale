#pragma once

#include "esp_err.h"
#include "i2c_protocol.h"

#define READER_I2C_BUS          I2C_NUM_1
#define READER_I2C_SCL          43
#define READER_I2C_SDA          44
#define READER_I2C_FREQ         400000
#define READER_POLL_MS          25

esp_err_t reader_comm_init(void);
esp_err_t reader_comm_poll(i2c_packet_t *pkt);
