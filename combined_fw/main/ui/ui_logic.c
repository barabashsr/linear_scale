#include "ui_logic.h"
#include "i2c_protocol.h"
#include "mcp23017.h"
#include "esp_log.h"
#include <string.h>

static state_t g_st;

void logic_init(void)
{
    memset(&g_st, 0, sizeof(g_st));
    g_st.axes[AXIS_RADIAL].radius_mode = true;
    g_st.diam_value = CFG_DIAM_DEFAULT_MM;
}

state_t *logic_get(void) { return &g_st; }

float logic_get_mm(axis_t a, int idx)
{
    int32_t d = (idx == -1) ? g_st.axes[a].main.delta_005mm : g_st.axes[a].sp[idx].delta_005mm;
    return position_to_mm_axis(d, (int)a);
}

float logic_get_display_mm(axis_t a, int idx)
{
    float v = logic_get_mm(a, idx);
    if (a == AXIS_RADIAL && g_st.axes[AXIS_RADIAL].radius_mode) v *= 2.0f;
    return v;
}

void logic_update(int32_t axial, int32_t radial)
{
    for (int i = 0; i < AXIS_COUNT; i++) {
        axis_data_t *ax = &g_st.axes[i];
        ax->raw_005mm = (i == AXIS_AXIAL) ? axial : radial;
        int32_t cur = ax->raw_005mm;
        ax->main.delta_005mm = cur - ax->main.ref_005mm;
        for (int j = 0; j < CFG_MAX_SETPOINTS; j++)
            if (ax->sp[j].active) ax->sp[j].delta_005mm = cur - ax->sp[j].ref_005mm;
    }
}

void logic_zero_main(axis_t a)
{
    axis_data_t *ax = &g_st.axes[a];
    int32_t cur = ax->raw_005mm;
    int32_t shift = cur - ax->main.ref_005mm;
    ax->main.ref_005mm = cur;
    ax->main.delta_005mm = 0;
    for (int i = 0; i < CFG_MAX_SETPOINTS; i++) {
        if (ax->sp[i].active) {
            ax->sp[i].ref_005mm += shift;
            ax->sp[i].delta_005mm = cur - ax->sp[i].ref_005mm;
        }
    }
}

void logic_zero_sp(axis_t a, int idx)
{
    if (idx < 0 || idx >= CFG_MAX_SETPOINTS) return;
    sp_t *sp = &g_st.axes[a].sp[idx];
    sp->ref_005mm = g_st.axes[a].raw_005mm;
    sp->delta_005mm = 0;
    sp->active = true;
}

void logic_toggle_rd(void)
{
    g_st.axes[AXIS_RADIAL].radius_mode = !g_st.axes[AXIS_RADIAL].radius_mode;
    mcp23017_set_led(CFG_MCP23017_ADDR, CFG_MCP_LED_RD,
                     g_st.axes[AXIS_RADIAL].radius_mode);
}

void logic_set_diameter(float mm)
{
    g_st.diam_value = mm;
    axis_data_t *ax = &g_st.axes[AXIS_RADIAL];
    int32_t target = mm_to_position(mm / (ax->radius_mode ? 2.0f : 1.0f));
    int32_t shift = target - ax->main.delta_005mm;
    ax->main.ref_005mm = ax->raw_005mm - target;
    ax->main.delta_005mm = target;
    for (int i = 0; i < CFG_MAX_SETPOINTS; i++)
        if (ax->sp[i].active) ax->sp[i].ref_005mm += shift;
}

float logic_get_angle_deg(void)
{
    float ratio = (float)CFG_SPINDLE_GEAR_NUM / (float)CFG_SPINDLE_GEAR_DEN;
    return (g_st.spindle_raw - g_st.spindle_z_ref) * 360.0f / CFG_SPINDLE_CPR * ratio;
}

void (*g_on_diameter_btn)(void);
void (*g_on_axial_zero_btn)(void);

void logic_handle_btn(uint16_t btns)
{
    uint16_t chg = btns & ~g_st.btn_state;
    if (chg) ESP_LOGI("btn", "btns=0x%04X chg=0x%04X", btns, chg);
    g_st.btn_state = btns;
    if (!chg) return;
    for (int i = 0; i < CFG_MAX_SETPOINTS; i++) {
        if (chg & (BTN_RADIAL_T1 << i)) logic_zero_sp(AXIS_RADIAL, i);
        if (chg & (BTN_AXIAL_T1 << i))  logic_zero_sp(AXIS_AXIAL, i);
    }
    if (chg & BTN_RD_TOGGLE) logic_toggle_rd();
    if (chg & BTN_DIAMETER && g_on_diameter_btn) g_on_diameter_btn();
    if (chg & BTN_AXIAL_ZERO && g_on_axial_zero_btn) g_on_axial_zero_btn();
    if (chg & BTN_DIAM_ZERO) logic_zero_main(AXIS_RADIAL);
    if (chg & BTN_DIAM_T1) logic_zero_sp(AXIS_RADIAL, 0);
    if (chg & BTN_DIAM_T2) logic_zero_sp(AXIS_RADIAL, 1);
}

void logic_set_rpm(float rpm) { g_st.spindle_rpm = rpm; }
float logic_get_rpm(void) { return g_st.spindle_rpm; }
void logic_zero_spindle(void) { g_st.spindle_z_ref = g_st.spindle_raw; }
