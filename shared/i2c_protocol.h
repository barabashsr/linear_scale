#pragma once

#include <stdint.h>
#include <stddef.h>

#define I2C_READER_ADDR         0x20
#define I2C_PACKET_SIZE         8

#define BTN_RADIAL_T1           (1 << 0)
#define BTN_RADIAL_T2           (1 << 1)
#define BTN_RADIAL_T3           (1 << 2)
#define BTN_RADIAL_T4           (1 << 3)
#define BTN_AXIAL_T1            (1 << 4)
#define BTN_AXIAL_T2            (1 << 5)
#define BTN_AXIAL_T3            (1 << 6)
#define BTN_AXIAL_T4            (1 << 7)

#define BTN_RD_TOGGLE           (1 << 8)
#define BTN_ENCODER_BTN         (1 << 9)

#define BTN_MASK_ALL            0x03FFUL

#define SCALE_RESOLUTION_MM     0.005
#define POSITION_MIN            (-2147483647)
#define POSITION_MAX            ( 2147483647)

#pragma pack(push, 1)
typedef struct {
    uint8_t  version;
    uint16_t btn_state;
    int32_t  position_005mm;
    uint8_t  crc;
} i2c_packet_t;
#pragma pack(pop)

static inline uint8_t i2c_packet_crc(const uint8_t *data, size_t len)
{
    uint8_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
    }
    return crc;
}

static inline float position_to_mm(int32_t pos)
{
    return pos * SCALE_RESOLUTION_MM;
}

static inline int32_t mm_to_position(float mm)
{
    return (int32_t)(mm / SCALE_RESOLUTION_MM);
}
