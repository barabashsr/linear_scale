# UI Vertical 3-Row Layout — Implementation Plan

## Layout (confirmed)
```
800×480 screen, 3 equal rows (~148px), 6px gaps, 8px top margin
Each row spans full width (800 - 2*OX - 2*PAD)

Row 1 (y=8):  RADIAL   — [⌀] [D/R] left, title+value+unit, 2 setpoints + [0] right
Row 2 (y=162): AXIAL   — [0] left, title+value+unit, 2 setpoints + [0] right
Row 3 (y=316): SPINDLE — no left buttons, title+value, 1 angle setpoint + [0], 1 RPM (no [0])
```

## Files to change

### 1. Generate ⌀ font
```bash
cd combined_fw/main/fonts
lv_font_conv --font Roboto-Bold.ttf --bpp 4 --size 32 \
  --no-compress --range 0x2300 \
  --format lvgl --output ui_font_roboto_bold_diam_32.c \
  --lv-font-name ui_font_roboto_bold_diam_32
# Fix include path if needed
```

### 2. Create main/buttons.h — update ax_ui_t struct (in ui_main.c)
No header change needed — `ax_ui_t` is in `ui_main.c`

### 3. Rewrite ui_main.c
Complete rewrite with 3-row vertical layout.

**Dimensions:**
- `ROW_H = 148`, `ROW_GAP = 6`, `TOP_MARGIN = 8`
- Left button col: x=8, stacked vertically, 56×56 each, 4px gap
- Center area: left of row plate, contains title + value+unit (inline)
- Right area: setpoint plates stacked vertically, 0 buttons at x=screen_right - 6 - 56
- Setpoint plate: 190px wide × 54px tall, 4px gap between stacked plates
- 0 button: 56×56 square, right-aligned near screen border

**Row plate backgrounds:**
- Each row: `lv_obj_create(scr)`, span full usable width
- Row plate style: `&s_panel` with pad=8
- Content starts at x=8+PAD inside plate

**ax_ui_t struct:**
```c
typedef struct {
    lv_obj_t *plate;        // row background
    lv_obj_t *title;        // axis title label
    lv_obj_t *val;          // main value + unit
    lv_obj_t *btn_left[2];  // left buttons (⌀+D/R or 0)
    lv_obj_t *sp_p[SP_C];   // setpoint plates
    lv_obj_t *sp_v[SP_C];   // setpoint value labels
    lv_obj_t *sp_b[SP_C];   // setpoint 0 buttons
} ax_ui_t;
```

**ui_main_update:**
- Each row: update title, main value (+unit inline), setpoint values
- Spindle: update angle, RPM (inline unit in angle, "ОБ/МИН" for RPM)

**Spindle specifics:**
- No left buttons
- 1 angle setpoint: "123.4°" with 0 button
- 1 RPM: "1500 ОБ/МИН" — no 0 button (separate label object)
- `g_sp_ang` and `g_sp_rpm` stored as globals (not in ax_ui_t)

### 4. Build & verify
```bash
idf.py build
```

## No changes to:
- config.h (layout defines already at good values)
- i2c_protocol.h, ui_logic.h/c, buttons.c, main.c, scale.c, spindle_enc.c
- ui_styles.h/c (styles already support the needed sizes)
