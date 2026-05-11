# Numpad Fix Plan

## Issues to fix

### 1. Remove debug label + cursor + active digit logic
- Delete `g_np_label`, `np_update_display()`, `g_np_cursor`, `g_edit_pos`, `np_cursor_move()`
- Simplify: internally track a local `edit_pos` in `np_place_digit`/`np_backspace` (static), 
  but no visual cursor displayed. User knows which digit changes by context.
- `show_numpad()` — remove the `lv_label_create(g_np_label)` block entirely

### 2. Fix BS button — use actual backspace symbol
- Generate 24px font from fa-solid.ttf with U+F55A (delete-left) or U+232B (⌫)
- Or use Montserrat 24 with Unicode ← symbol
- Change button text from "BS" to the symbol, use different font for that one button

### 3. Fix slider — 0–300 mm, 0.1 mm resolution
- Local constant: `NP_SLIDER_MAX 300.0f`, `NP_SLIDER_RES 3000`
- Slider range 0–3000, each step = 0.1 mm
- Show value with 1 decimal by default (slider state)
- Digits add precision: 2nd decimal, then 3rd

### 4. Fix modal width — 460 px
- 4×4 grid: 3×76 + 158 + 3×6 = 404 px + 28 px margins = ~432 → 460 px total
- Modal: `lv_obj_set_size(g_modal, 460, 370)`

### 5. Fix layout — title/value overlap
- Title: `LV_ALIGN_TOP_LEFT, 12, 8` (unchanged)
- Value container: explicit position `x=248, y=8, w=200, h=48`
  (460 - 12 - 200 = 248 → right side of modal with 12px margin)

### 6. Simplify digit entry
- Slider: shows "23.4" (1 decimal in the display)  
- Type digit: fills 2nd decimal → 3rd → overwrites 3rd
- Backspace: clears 3rd → 2nd → reverts to 1 decimal (never clears 1st)
- Clear: resets to slider value (1 decimal)

## Files to change
- `ui/ui_main.c` only — remove cursor/debug code, fix slider range, fix width, fix layout, fix BS text
