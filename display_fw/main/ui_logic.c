#include "ui_logic.h"
#include "i2c_protocol.h"
#include <string.h>
#include <string.h>
#include <math.h>

static app_state_t g_state;

void ui_logic_init(void)
{
    memset(&g_state, 0, sizeof(g_state));
    g_state.axes[AXIS_RADIAL].radius_mode = true;

    for (int a = 0; a < AXIS_COUNT; a++) {
        g_state.axes[a].main.active = true;
        for (int i = 0; i < MAX_SETPOINTS; i++) {
            g_state.axes[a].sp[i].active = false;
        }
    }
}

app_state_t *ui_logic_get_state(void)
{
    return &g_state;
}

float sp_get_mm(axis_t axis, int idx)
{
    int32_t delta;
    if (idx == -1) {
        delta = g_state.axes[axis].main.delta_005mm;
    } else {
        delta = g_state.axes[axis].sp[idx].delta_005mm;
    }
    return position_to_mm(delta);
}

float sp_get_mm_radius(axis_t axis, int idx)
{
    float v = sp_get_mm(axis, idx);
    if (axis == AXIS_RADIAL && g_state.axes[AXIS_RADIAL].radius_mode) {
        return v;
    }
    return v;
}

void ui_logic_zero_main(axis_t axis)
{
    axis_data_t *ax = &g_state.axes[axis];

    int32_t current_pos;
    if (axis == AXIS_AXIAL) {
        current_pos = g_state.raw_axial_005mm;
    } else {
        current_pos = ax->main.ref_pos_005mm + ax->main.delta_005mm;
    }

    int32_t shift = current_pos - ax->main.ref_pos_005mm;
    ax->main.ref_pos_005mm = current_pos;
    ax->main.delta_005mm = 0;

    for (int i = 0; i < MAX_SETPOINTS; i++) {
        if (ax->sp[i].active) {
            ax->sp[i].ref_pos_005mm += shift;
            ax->sp[i].delta_005mm = current_pos - ax->sp[i].ref_pos_005mm;
        }
    }
}

void ui_logic_zero_setpoint(axis_t axis, int idx)
{
    if (idx < 0 || idx >= MAX_SETPOINTS) return;
    axis_data_t *ax = &g_state.axes[axis];
    setpoint_t *sp = &ax->sp[idx];

    int32_t current_pos;
    if (axis == AXIS_AXIAL) {
        current_pos = g_state.raw_axial_005mm;
    } else {
        current_pos = ax->main.ref_pos_005mm + ax->main.delta_005mm;
    }

    sp->ref_pos_005mm = current_pos;
    sp->delta_005mm = 0;
    sp->active = true;
}

void ui_logic_set_radial_main(int32_t pos)
{
    axis_data_t *ax = &g_state.axes[AXIS_RADIAL];
    int32_t shift = pos - ax->main.ref_pos_005mm;
    ax->main.ref_pos_005mm = pos;
    ax->main.delta_005mm = 0;

    for (int i = 0; i < MAX_SETPOINTS; i++) {
        if (ax->sp[i].active) {
            ax->sp[i].ref_pos_005mm += shift;
        }
    }
}

void ui_logic_toggle_rd_mode(void)
{
    g_state.axes[AXIS_RADIAL].radius_mode =
        !g_state.axes[AXIS_RADIAL].radius_mode;
}

void ui_logic_update_position(int32_t axial_pos)
{
    g_state.raw_axial_005mm = axial_pos;

    for (int a = 0; a < AXIS_COUNT; a++) {
        axis_data_t *ax = &g_state.axes[a];
        int32_t current;

        if (a == AXIS_AXIAL) {
            current = axial_pos;
        } else {
            current = ax->main.ref_pos_005mm + ax->main.delta_005mm;
        }

        ax->main.delta_005mm = current - ax->main.ref_pos_005mm;

        for (int i = 0; i < MAX_SETPOINTS; i++) {
            if (ax->sp[i].active) {
                ax->sp[i].delta_005mm = current - ax->sp[i].ref_pos_005mm;
            }
        }
    }
}

void ui_logic_handle_button(uint16_t btn_state)
{
    uint16_t changed = btn_state & ~g_state.btn_state;
    g_state.btn_state = btn_state;
    if (!changed) return;

    for (int i = 0; i < MAX_SETPOINTS; i++) {
        if (changed & (BTN_RADIAL_T1 << i)) {
            ui_logic_zero_setpoint(AXIS_RADIAL, i);
        }
        if (changed & (BTN_AXIAL_T1 << i)) {
            ui_logic_zero_setpoint(AXIS_AXIAL, i);
        }
    }
    if (changed & BTN_RD_TOGGLE) {
        ui_logic_toggle_rd_mode();
    }
}
