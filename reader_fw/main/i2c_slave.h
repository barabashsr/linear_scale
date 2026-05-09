#pragma once

#include "esp_err.h"
#include "i2c_protocol.h"

#define I2C_SLAVE_NUM           I2C_NUM_0
#define I2C_SLAVE_SCL_IO        26
#define I2C_SLAVE_SDA_IO        27
#define I2C_SLAVE_RX_BUF_LEN    8
#define I2C_SLAVE_TX_BUF_LEN    I2C_PACKET_SIZE

esp_err_t i2c_slave_init(void);
