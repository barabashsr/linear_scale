# UI Layout Redesign Plan

## Goals
1. Remove "главная" labels on both sides
2. Radial: replace "ОБНУЛИТЬ" with "ДИАМЕТР" button (opens numpad), D/R stays next to it
3. Remove separate diameter panel at bottom
4. Setpoint "0" buttons moved OUTSIDE plates: LEFT for radial, RIGHT for axial
5. No T1/T2 labels on setpoint plates — just the value number
6. Spindle: same height as setpoint (70px), red digits, 0 button on LEFT
7. All panels: scrollbar mode OFF

## Layout

```
╔═════════════════════════╦══╦═════════════════════════╗
║  РАДИАЛЬНАЯ            ║  ║  ОСЕВАЯ                 ║ ← title 30h
║  +123.456              ║  ║  +78.900                ║ ← main 72px 80h
║                        ║  ║                         ║
║ ┌──────────┬─────────┐ ║  ║ ┌─────────────────────┐ ║
║ │ ДИАМЕТР  │  D/R    │ ║  ║ │      ОБНУЛИТЬ       │ ║ ← btn row 52h
║ └──────────┴─────────┘ ║  ║ └─────────────────────┘ ║
║                        ║  ║                         ║
║ ┌──┐ ┌───────────────┐ ║  ║ ┌───────────────┐ ┌──┐ ║
║ │0 │ │   +12.345     │ ║  ║ │   +12.345     │ │0 │ ║ ← SP1 70h
║ └──┘ └───────────────┘ ║  ║ └───────────────┘ └──┘ ║
║ ┌──┐ ┌───────────────┐ ║  ║ ┌───────────────┐ ┌──┐ ║
║ │0 │ │   +45.678     │ ║  ║ │   +45.678     │ │0 │ ║ ← SP2 70h
║ └──┘ └───────────────┘ ║  ║ └───────────────┘ └──┘ ║
║                        ║  ║                         ║
╠══════════════════════════════════════════════════════╣
║ ┌─────┐ ┌─────────────────────────────────────────┐ ║
║ │  0  │ │ ШПИНДЕЛЬ    123.4°    1500 ОБ/МИН       │ ║ ← spdl 70h
║ └─────┘ └─────────────────────────────────────────┘ ║
╚══════════════════════════════════════════════════════╝
```

## Files to change

### ui_main.c — complete rewrite

1. **ax_ui_t struct** — remove `lbl`, `diam_pan`, `diam_btn`, `sp_l[]`:
```c
typedef struct {
    lv_obj_t *title, *val, *btn_main, *btn_rd;
    lv_obj_t *sp_p[SP_C], *sp_v[SP_C], *sp_b[SP_C];
} ax_ui_t;
```

2. **build_axis** — new layout:
   - Remove "главная" label section
   - Radial button row: `[ДИАМЕТР 240px] [D/R 112px]` — diam opens numpad
   - Axial button row: `[ОБНУЛИТЬ full-width]` — opens confirm dialog
   - Setpoint rows: 0 button (48×70) + plate (310×70) side by side
     - Radial: `[0] [plate]` — 0 on left
     - Axial: `[plate] [0]` — 0 on right
   - 0 buttons are children of SCREEN (not inside plates)
   - Plate contains only value label, left-aligned with 8px padding
   - All panels: `lv_obj_set_scrollbar_mode(LV_SCROLLBAR_MODE_OFF)`

3. **ui_main_create** — spindle restructured:
   - Spindle 0 button: child of screen, 48×70, positioned at bottom-left
   - Spindle panel: child of screen, 70px tall, full-width (minus button), to the right of 0 button
   - Angle label uses `s_val_act` (red highlight)
   - RPM label uses `s_val_act` (red highlight)

4. **upd_axis** — simplified:
   - Remove `lbl` update
   - Setpoint: only update value label (no T-label)
   - Active setpoints: red text, inactive: dim + "---.---"

5. **ui_main_update** — spindle values use `g_sp_ang`, `g_sp_rpm` pointers (red style applied in create)

### No changes to:
- config.h (already has CFG_MAX_SETPOINTS=2)
- i2c_protocol.h
- ui_logic.h/c
- buttons.c
- main.c (already has 1ms loop + lvgl_flush_all)
- scale.c / spindle_enc.c

## Build & verify
- `idf.py build` — expect zero errors
