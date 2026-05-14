# Button + LED + UI Update Plan

## 1. config.h — add MCP23017 GPIOB config

```c
/* ── MCP23017 GPIOB (buttons + LED) ──────────── */
#define CFG_BTN_MCP_DIAM_T1     0    // GPIOB0 — radial T1 setpoint
#define CFG_BTN_MCP_DIAM_T2     1    // GPIOB1 — radial T2 setpoint
#define CFG_LED_MCP_RD          2    // GPIOB2 — R/D mode LED (HIGH = diameter)
```

## 2. i2c_protocol.h — new button masks

```c
#define BTN_DIAM_ZERO           (1 << 8)
#define BTN_DIAM_T1             (1 << 9)   // MCP GPIOB0
#define BTN_DIAM_T2             (1 << 10)  // MCP GPIOB1
```

## 3. buttons.c — extend for AW pins 6-7 + MCP GPIOB

- Add `CFG_BTN_AW_AXIAL_T2` (pin 6) and `CFG_BTN_AW_DIAM_ZERO` (pin 7) to aw_pins/aw_masks
- Read MCP23017 GPIOB register for GPB0/GPB1 button states
- Merge into raw bitmap with BTN_DIAM_T1/BTN_DIAM_T2 masks

## 4. mcp23017.c — configure GPIOB + add read helper

```c
// After GPIOA config:
mcp23017_write(addr, MCP_IODIRB, 0x03);  // GPB0-1=input, GPB2-7=output
mcp23017_write(addr, MCP_GPPUB, 0x03);   // pull-ups on GPB0-1
mcp23017_write(addr, MCP_OLATB, 0x00);   // LED off, outputs LOW
```

New function in `mcp23017.h`:
```c
esp_err_t mcp23017_read_gpiob(uint8_t addr, uint8_t *val);
```

But wait — mcp23017_read already exists and can read any register. Just call `mcp23017_read(addr, 0x13, &val)` to read GPIOB.

Actually, let's keep it simple — add a read wrapper or just use the existing `mcp23017_read` function:
```c
uint8_t gpiob;
mcp23017_read(CFG_MCP23017_ADDR, MCP_GPIOB, &gpiob);
```

## 5. mcp23017.h — expose MCP registers for external use

The macros already exist. `mcp23017_read`/`mcp23017_write` are already declared.

Add LED helper:
```c
void mcp23017_set_led(uint8_t addr, uint8_t pin, bool on);
```

## 6. ui_logic.c — handle new buttons + LED toggle

```c
if (chg & BTN_DIAM_ZERO) logic_zero_main(AXIS_RADIAL);
if (chg & BTN_DIAM_T1) logic_zero_sp(AXIS_RADIAL, 0);
if (chg & BTN_DIAM_T2) logic_zero_sp(AXIS_RADIAL, 1);
// BTN_RADIAL_T1/T2 already handled by existing loop
```

`logic_toggle_rd()`:
```c
void logic_toggle_rd(void) {
    g_st.axes[AXIS_RADIAL].radius_mode = !g_st.axes[AXIS_RADIAL].radius_mode;
    uint8_t led = g_st.axes[AXIS_RADIAL].radius_mode ? (1 << CFG_LED_MCP_RD) : 0;
    mcp23017_write(CFG_MCP23017_ADDR, MCP_OLATB, led);
}
```

## 7. ui_main.c — UI changes

### Radial row — add diam zero button
Radial left buttons (3 stacked, 44×44 to fit):
- ⌀ (diameter numpad) — top
- D/R (toggle) — middle  
- 0 (diam zero) — bottom → calls `logic_zero_main(AXIS_RADIAL)`

### Spindle row — replace RPM with diam T2
Remove:
- `g_sp_rpm_v`, `g_sp_rpm_u` labels
- RPM plate creation

Replace with:
- Diameter T2 setpoint plate (same style as T1/T2 in radial row)
- "0" button that calls `logic_zero_sp(AXIS_RADIAL, 1)` (same as radial T2)
- Value updates from `logic_get_display_mm(AXIS_RADIAL, 1)` in `ui_main_update`

### ui_main_update changes
```c
// Remove:
//   snprintf(buf, sizeof(buf), "%.0f", (double)logic_get_rpm());
//   lv_label_set_text(g_sp_rpm_v, buf);

// Keep spindle angle update. Add diam T2 update:
//   fpos(buf, ..., logic_get_display_mm(AXIS_RADIAL, 1));
//   lv_label_set_text(g_sp_diam_t2_v, buf);
```

## Files changed
- config.h — add MCP GPIOB config
- i2c_protocol.h — add BTN_DIAM_ZERO/T1/T2
- buttons.c — add AW pins 6-7 + MCP GPIOB read
- mcp23017.c — add GPIOB config + LED helper
- mcp23017.h — expose LED helper
- ui_logic.c — handle new buttons + LED in toggle
- ui_main.c — radial 3-btn left column, spindle T2 replaces RPM
