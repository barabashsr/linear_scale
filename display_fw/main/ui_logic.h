#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_SETPOINTS 4

typedef enum {
    AXIS_RADIAL = 0,
    AXIS_AXIAL  = 1,
    AXIS_COUNT
} axis_t;

typedef struct {
    int32_t ref_pos_005mm;
    int32_t delta_005mm;
    bool    active;
} setpoint_t;

typedef struct {
    setpoint_t main;
    setpoint_t sp[MAX_SETPOINTS];
    bool       radius_mode;
} axis_data_t;

typedef struct {
    int32_t  raw_axial_005mm;
    axis_data_t axes[AXIS_COUNT];
    uint16_t btn_state;
    bool     reader_ok;
} app_state_t;

void ui_logic_init(void);
app_state_t *ui_logic_get_state(void);
float sp_get_mm(axis_t axis, int idx);
float sp_get_mm_radius(axis_t axis, int idx);

void ui_logic_zero_main(axis_t axis);
void ui_logic_zero_setpoint(axis_t axis, int idx);
void ui_logic_set_radial_main(int32_t pos);
void ui_logic_toggle_rd_mode(void);
void ui_logic_update_position(int32_t axial_pos);

void ui_logic_handle_button(uint16_t btn_state);
