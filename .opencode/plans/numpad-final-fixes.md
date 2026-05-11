# Numpad Final Fixes Plan

## 1. Fix sign flip on OK (root cause)
`logic_set_diameter()` forces `delta_005mm = 0`, so display always shows 0 after OK.
**Fix** (`ui/ui_logic.c`): set ref such that delta equals the target value.

```c
void logic_set_diameter(float mm) {
    g_st.diam_value = mm;
    axis_data_t *ax = &g_st.axes[AXIS_RADIAL];
    int32_t target = mm_to_position(mm / (ax->radius_mode ? 2.0f : 1.0f));
    int32_t shift = target - ax->main.delta_005mm;
    ax->main.ref_005mm = ax->raw_005mm - target;
    ax->main.delta_005mm = target;
    for (int i = 0; i < CFG_MAX_SETPOINTS; i++)
        if (ax->sp[i].active) ax->sp[i].ref_005mm += shift;
}
```

## 2. Remove slider entirely
- Delete `np_slider_cb()`
- Delete slider creation block in `show_numpad()`
- Remove `g_numpad_val` usage in `show_numpad()` (load from axis, store in buffer only)
- No more sync issues

## 3. Fix leading zero on digit entry
When buffer is `"+0"` or `"-0"` (len==2), REPLACE the `'0'` with the digit instead of appending.
`"+0"` + `'1'` → `"+1"`, `"+1"` + `'2'` → `"+12"`, `"+123"` + `'4'` → auto-dot → `"+123.4"`

## 4. Clear → `"+0"` (not `"+0.0"`)

## 5. Block horizontal scroll
Add `lv_obj_clear_flag(g_modal, LV_OBJ_FLAG_SCROLLABLE)` in `show_numpad()`

## 6. Adjust layout (no slider)
```
  y=8   ДИАМЕТР, mm
  y=36    +123.456
  y=80    [1][2][3] [OK]
          [4][5][6] [Отмена]
          [7][8][9] [C]
          [.][0][±] [⌫]
```
Modal: 460×290

## Files changed
- `ui/ui_logic.c` — fix `logic_set_diameter`
- `ui/ui_main.c` — remove slider, fix digit entry, fix clear, block scroll, adjust layout
