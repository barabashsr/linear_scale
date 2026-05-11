# Numpad Modal Redesign Plan

## Layout

```
┌─────────────────────────────────────────────────────────┐
│                   UNDERLAY (40% black)                   │
│              click → close modal (discard)               │
│                                                          │
│   ┌──────────────────────────────────┐                  │
│   │ ДИАМЕТР, mm       +23.456       │  ← title L / val R│
│   │                                  │                  │
│   │  ├───────────○───────────┤       │  ← slider        │
│   │                                  │                  │
│   │  ┌───┬───┬───┬──────────┐       │                  │
│   │  │ 1 │ 2 │ 3 │    OK    │       │  ← 4×4 keypad    │
│   │  ├───┼───┼───┼──────────┤       │                  │
│   │  │ 4 │ 5 │ 6 │  Cancel  │       │                  │
│   │  ├───┼───┼───┼──────────┤       │                  │
│   │  │ 7 │ 8 │ 9 │  Clear   │       │                  │
│   │  ├───┼───┼───┼──────────┤       │                  │
│   │  │ . │ 0 │ ± │    ⌫    │       │                  │
│   │  └───┴───┴───┴──────────┘       │                  │
│   └──────────────────────────────────┘                  │
└─────────────────────────────────────────────────────────┘
```

## Entry Logic

| Action | Effect | edit_pos |
|--------|--------|----------|
| Slider drag | Changes integer+1st decimal in 0.1mm steps | reset to 0 |
| Digit press | Places digit at edit_pos | 0→1, 1→2, 2→2 |
| ⌫ Backspace | Clears digit at edit_pos, retreats | 2→1, 1→0, 0→remove last int digit |
| C Clear | Resets to last slider value | 0 |
| ± Toggle | Flips sign | unchanged |
| . Decimal | Appends decimal point if not present | — |

## Active Digit Indicator

Value displayed as 6 separate labels side by side:
```
[+] [23] [.] [4] [5] [6]
```
Active digit (where next keypress goes) changes color:
- edit_pos==0: all digits normal (slider controls)
- edit_pos==1: 2nd decimal digit → scarlet highlight + small cursor underline
- edit_pos==2: 3rd decimal digit → scarlet highlight + small cursor underline

## Underlay

Shared helper function `underlay_create(close_cb)`:
- Full-screen object at lv_scr_act()
- 40% black opacity
- LV_EVENT_CLICKED → calls close_cb
- Used by both numpad modal and confirm dialog

## Files to change

1. `ui/ui_main.c`:
   - Add `underlay_create()` and modify `show_numpad()`
   - Replace keypad grid with 4×4 layout matching KEYPAD.md
   - Add segmented value labels (sign, int, dot, d1, d2, d3)
   - Add edit positioning logic
   - Modify `show_confirm()` to use underlay
   - Font: value digits use 48px Roboto Bold for readability

2. No changes to main.c, config.h, CMakeLists.txt
