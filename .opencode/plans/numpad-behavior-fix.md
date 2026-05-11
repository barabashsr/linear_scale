# Numpad Behavior Fix Plan

## 1. Buffer model
`g_numpad_buf[16]` = display string with sign, e.g. `"+123.456"` or `"-45.3"`
`g_numpad_val` = float parsed from buffer (only used for slider init)

## 2. Backspace — calculator style
```
"+245.345" → BS → "+245.34" → BS → "+245.3" → BS → "+245" → BS → "+24" → BS → "+2" → BS → "+0"
```
- Always removes last character
- If removing last digit leaves `". "` → also remove the dot
- Never goes below "+0" (minimum)

## 3. Digit entry — auto-insert dot after 3rd digit
```
type "1"     → "+1"
type "2"     → "+12"  
type "3"     → "+123"
type "4"     → "+123.4"    ← dot auto-inserted
type "5"     → "+123.45"
type "6"     → "+123.456"
type "7"     → "+123.457"  ← 3rd decimal overwrites
```
- If no dot and buffer reaches 4 chars after sign, insert `'.'` between chars 3 and 4
- Cap decimal places at 3

## 4. Clear → sets value to "+0.0"
Not reset to slider value. Just `g_numpad_buf = "+0.0"`.

## 5. +/− → flips the first character
`'+' ↔ '−'`, preserves rest of buffer.

## 6. Slider — preserves sign, changes integer + 1st decimal
- Read slider 0–3000 → mm = slider × 0.1
- Keep current sign from `g_numpad_buf[0]`
- Set buffer to `"{sign}{mm with 1 decimal}"` (e.g., `"−23.4"`)
- If mm ends in `.0`, show as `"−23.0"` (always 1 decimal from slider)

## 7. Don't affect axis until OK
- Remove ALL `logic_set_diameter()` calls from: np_place_digit, np_backspace, np_clear, np_toggle_sign, np_slider_cb
- Only call `logic_set_diameter(editing_value)` in OK handler
- Store editing value as float, apply on OK

## 8. Modal loads from axis only once
- `show_numpad()`: `g_numpad_val = logic_get()->diam_value`, init buffer correctly
- Modal edits are local to `g_numpad_buf`
- On OK: `logic_set_diameter(atof(g_numpad_buf))`

## 9. Rename Cancel → Отмена
Change button text: `"Cancel"` → `"\u041E\u0442\u043C\u0435\u043D\u0430"`

## 10. 10px gap value→slider
Slider y: 92 → 102

## 11. Opposing signs fix
The issue was `np_set_value` storing absolute value in buffer but showing signed in label. New unified buffer approach fixes this — buffer IS the display string with sign.

## Files changed
- `ui/ui_main.c` only — numpad functions rewrite
