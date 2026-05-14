# Button Layout Fix + Debug Plan

## Issues

### 1. Radial left buttons — wrong
Current: ⌀, D/R, 0 (3 buttons)  
Correct: ⌀, D/R (2 buttons, back to original). The "0" was meant for spindle.

### 2. Spindle left button — missing
Add 66×66 "0" button in COL1 for spindle row, calls `spindle_zero_cb`, aligned with angle setpoint Y.

### 3. R/D label — overlaps title
"РАДИУС"/"ДИАМЕТР" (9 chars) too wide. Change to single bold letter "R"/"D" using same UI_FONT_DIAM (32px) or larger.

### 4. Debug logging — back on
Re-add to `buttons.c` (AW9523 raw per button press) and `ui_logic.c` (btns/chg/state).

### 5. AW9523 — no buttons detected
Root cause unknown. Steps:
- Verify GCR register address (0x12) is correct for this chip — may need 0x11 (CTL) instead
- Try per-pin pull-up via CTL register 0x11 bit settings
- Add debug log of raw port0 reads every 500ms during idle

### 6. MCP23017 GPIOB — cascading failure
If `mcp23017_init` fails (no chip), all GPIOB config is skipped. Fix: decouple — init GPIOB with direct I2C writes even if keypad not found. Read OLATB before modify in LED toggle.

### 7. LED — doesn't work
Same root cause as #6. Also add read-before-modify to `mcp23017_set_led` to avoid clobbering other GPIOB outputs.

## Files changed
- `ui_main.c` — radial: remove 0 from left, restore D/R; spindle: add 0; R/D→single letter
- `buttons.c` — re-add debug logging
- `ui_logic.c` — re-add debug logging
- `mcp23017.c` — decouple GPIOB init, read-before-modify OLATB
- `aw9523.c` — try register 0x11, add periodic debug read
