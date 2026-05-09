#include "ui_logic.h"
#include "i2c_protocol.h"
#include <string.h>

static app_state_t g_state;

void ui_logic_init(void)
{
    memset(&g_state, 0, sizeof(g_state));
    g_state.axes[AXIS_RADIAL].radius_mode = true;
}

app_state_t *ui_logic_get_state(void) { return &g_state; }

float sp_get_mm(axis_t axis, int idx)
{
    int32_t delta = (idx == -1) ? g_state.axes[axis].main.delta_005mm
                                : g_state.axes[axis].sp[idx].delta_005mm;
    return position_to_mm(delta);
}

void ui_logic_zero_main(axis_t axis)
{
    axis_data_t *ax = &g_state.axes[axis];
    int32_t cur = ax->raw_pos_005mm;
    int32_t shift = cur - ax->main.ref_pos_005mm;
    ax->main.ref_pos_005mm = cur;
    ax->main.delta_005mm = 0;
    for (int i = 0; i < CFG_MAX_SETPOINTS; i++) {
        if (ax->sp[i].active) {
            ax->sp[i].ref_pos_005mm += shift;
            ax->sp[i].delta_005mm = cur - ax->sp[i].ref_pos_005mm;
        }
    }
}

void ui_logic_zero_setpoint(axis_t axis, int idx)
{
    if (idx < 0 || idx >= CFG_MAX_SETPOINTS) return;
    setpoint_t *sp = &g_state.axes[axis].sp[idx];
    sp->ref_pos_005mm = g_state.axes[axis].raw_pos_005mm;
    sp->delta_005mm = 0;
    sp->active = true;
}

void ui_logic_set_radial_main(int32_t pos)
{
    axis_data_t *ax = &g_state.axes[AXIS_RADIAL];
    int32_t shift = pos - ax->main.ref_pos_005mm;
    ax->main.ref_pos_005mm = pos;
    ax->main.delta_005mm = 0;
    for (int i = 0; i < CFG_MAX_SETPOINTS; i++) {
        if (ax->sp[i].active) ax->sp[i].ref_pos_005mm += shift;
    }
}

void ui_logic_toggle_rd_mode(void)
{
    g_state.axes[AXIS_RADIAL].radius_mode = !g_state.axes[AXIS_RADIAL].radius_mode;
}

void ui_logic_update_positions(int32_t axial_pos, int32_t radial_pos)
{
    g_state.axes[AXIS_AXIAL].raw_pos_005mm = axial_pos;
    g_state.axes[AXIS_RADIAL].raw_pos_005mm = radial_pos;
    for (int a = 0; a < AXIS_COUNT; a++) {
        axis_data_t *ax = &g_state.axes[a];
        int32_t cur = ax->raw_pos_005mm;
        ax->main.delta_005mm = cur - ax->main.ref_pos_005mm;
        for (int i = 0; i < CFG_MAX_SETPOINTS; i++) {
            if (ax->sp[i].active) ax->sp[i].delta_005mm = cur - ax->sp[i].ref_pos_005mm;
        }
    }
}

void ui_logic_handle_button(uint16_t btn_state)
{
    uint16_t changed = btn_state & ~g_state.btn_state;
    g_state.btn_state = btn_state;
    if (!changed) return;
    for (int i = 0; i < CFG_MAX_SETPOINTS; i++) {
        if (changed & (BTN_RADIAL_T1 << i)) ui_logic_zero_setpoint(AXIS_RADIAL, i);
        if (changed & (BTN_AXIAL_T1 << i))  ui_logic_zero_setpoint(AXIS_AXIAL, i);
    }
    if (changed & BTN_RD_TOGGLE) ui_logic_toggle_rd_mode();
}

void ui_logic_zero_spindle(void) { g_state.spindle_ref = g_state.spindle_count; }

float sp_get_angle_deg(void)
{
    return (g_state.spindle_count - g_state.spindle_ref) * 360.0f / CFG_SPINDLE_CPR;
}
