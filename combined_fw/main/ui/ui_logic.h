#pragma once

#include "config.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum { AXIS_RADIAL = 0, AXIS_AXIAL = 1, AXIS_COUNT } axis_t;

typedef struct {
    int32_t ref_005mm;
    int32_t delta_005mm;
    bool    active;
} sp_t;

typedef struct {
    sp_t    main;
    sp_t    sp[CFG_MAX_SETPOINTS];
    bool    radius_mode;
    int32_t raw_005mm;
} axis_data_t;

typedef struct {
    axis_data_t axes[AXIS_COUNT];
    int32_t  spindle_raw;
    int32_t  spindle_z_ref;
    float    spindle_rpm;
    uint16_t btn_state;
    bool     modal_open;
    float    diam_value;
} state_t;

void   logic_init(void);
state_t *logic_get(void);
float  logic_get_mm(axis_t a, int idx);
float  logic_get_display_mm(axis_t a, int idx);
float  logic_get_angle_deg(void);
void   logic_update(int32_t axial, int32_t radial);
void   logic_zero_main(axis_t a);
void   logic_zero_sp(axis_t a, int idx);
void   logic_toggle_rd(void);
void   logic_set_diameter(float mm);
void   logic_handle_btn(uint16_t btns);
void   logic_set_rpm(float rpm);
void   logic_zero_spindle(void);
float  logic_get_rpm(void);

extern void (*g_on_diameter_btn)(void);
extern void (*g_on_axial_zero_btn)(void);
