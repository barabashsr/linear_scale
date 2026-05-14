#pragma once

#include "config.h"
#include <stdint.h>
#include <stddef.h>

#define BTN_RADIAL_T1           (1 << 0)
#define BTN_RADIAL_T2           (1 << 1)
#define BTN_AXIAL_T1            (1 << 2)
#define BTN_AXIAL_T2            (1 << 3)
#define BTN_RD_TOGGLE           (1 << 4)
#define BTN_AXIAL_ZERO          (1 << 5)
#define BTN_DIAMETER            (1 << 6)
#define BTN_DIAM_ZERO           (1 << 7)
#define BTN_DIAM_T1             (1 << 8)
#define BTN_DIAM_T2             (1 << 9)

static inline float position_to_mm(int32_t pos)
{
    return pos * CFG_SCALE_MM_PER_COUNT;
}

static inline float position_to_mm_axis(int32_t pos, int axis)
{
    return pos * (axis == 0 ? CFG_SCALE_RADIAL_MM : CFG_SCALE_AXIAL_MM);
}

static inline int32_t mm_to_position(float mm)
{
    return (int32_t)(mm / CFG_SCALE_MM_PER_COUNT);
}
