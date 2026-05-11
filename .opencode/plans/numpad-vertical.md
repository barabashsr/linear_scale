# Numpad Vertical Layout Fix

## Changes

### 1. Replace segmented labels with single label
- Remove: `g_vsign`, `g_vint`, `g_vdot`, `g_vd1`, `g_vd2`, `g_vd3` globals
- Add: single `g_np_label` (48px Roboto Bold, right-aligned)
- `np_set_value()` → format `+23.456` with `lv_label_set_text(g_np_label, buf)`
- Remove sign toggle label updates (just update the buffer string)
- Remove decimal digit individual updates

### 2. Vertical stacking layout
```
Modal 460×340, centered on screen

  y=8   ┌─────────────────────────────────┐
  y=8   │ ДИАМЕТР, mm                     │ ← title, left-aligned, 28px
  y=36  │                    +23.456      │ ← value, right-aligned, 48px
  y=80  │ ├─────────○─────────┤          │ ← slider, full width 430px
  y=110 │  1   2   3   OK                │
        │  4   5   6   Cancel            │ ← 4×4 grid, 82×46 buttons, 6px gap
        │  7   8   9   Clear             │
        │  .   0   ±   ⌫                │
        └─────────────────────────────────┘
```

### 3. Simplified functions
- `np_set_value()`: `snprintf` → `lv_label_set_text`
- `np_toggle_sign()`: flip `-`/`+` in buffer, `lv_label_set_text`
- `np_slider_cb()`: build string, `lv_label_set_text`
- Remove all `g_vsign`/`g_vint`/`g_vdot`/`g_vd*` set operations

### 4. Code changes
- Delete ~70 lines (segmented label creation, individual digit updates, cursor logic)
- Add ~5 lines (single label creation, simpler set_value)
- Net: ~65 lines removed

## Files
- `ui/ui_main.c` only
