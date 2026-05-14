# Button Debugging Plan

## Issues found

### 1. AW9523 oscillating (0x10 ↔ 0x00 every ~500ms)
- GCR (0x12=0x10) and CTL (0x11=0xFF) writes may be wrong/misconfigured
- Alternating pattern suggests AW9523 is resetting, or port0 being driven by LCD reset sequence
- Possible cause: `aw9523_set_output` for LCD pins (port1) uses static state cache initialized to 0 — but this shouldn't affect port0
- External pull-ups on board should make pins read HIGH when unpressed, but 0x00 reads suggest pull-ups not present or not working

### 2. MCP GPIOB buttons not responding
- LED output works (same port B, pin 2) — so GPIOB is being configured
- But button reads (GPB0/GPB1) always return unpressed
- Need to verify: raw GPIOB register value, whether read succeeds

### 3. R/D label — character not rendered + overlap
- Used `UI_FONT_DIAM` (only has U+00D8 / Ø) — "D" (U+0044) not in this font
- Fix: use `UI_FONT_TITLE` (28px Roboto, has Latin + Cyrillic)

### 4. Button logic not traceable
- No visibility into stable map or chg values
- Need comprehensive per-step logging

## Fix Plan

### Step 1: Fix R/D label
- Change font from `UI_FONT_DIAM` to `UI_FONT_TITLE` (has both "D" and "R")
- Move from `lv_obj_align_to` to explicit `lv_obj_set_pos` to avoid overlap:
  ```c
  lv_obj_set_pos(g_rd_label, 8 + text_width_of_title_approximate, TITLE_Y);
  ```
  Or use:
  ```c
  lv_obj_add_style(g_rd_label, &s_title, 0);
  lv_obj_align_to(g_rd_label, u->title, LV_ALIGN_OUT_RIGHT_MID, 4, 0);
  ```

### Step 2: Simplify AW9523 init — remove GCR + CTL
Revert to simple input config:
```c
// No GCR write, no CTL write
// Just: CFG_PORT0=0xFF, CFG_PORT1=0xFF
// Let board external pull-ups handle idle state
```
If board has external pull-ups, pins will read HIGH when unpressed. If pins still float, add external resistors.

### Step 3: Add comprehensive debug logging

**buttons.c — every read:**
```c
ESP_LOGI(TAG_BT, "AW=0x%02X MCP_GPIOB=0x%02X raw=0x%04X stable=0x%04X", 
         aw_raw, gb_raw, raw, stable);
```

**ui_logic.c — on edge:**
```c
if (chg) ESP_LOGI("btn", "btns=0x%04X chg=0x%04X st=0x%04X", btns, chg, g_st.btn_state);
```

**mcp23017.c — on GPIOB read:**
```c
ESP_LOGI("mcp", "GPIOB=0x%02X", val);
```

### Step 4: Verify MCP GPIOB read
- If MCP23017 not found (keypad_init fails), `mcp23017_init_gpiob` still configures GPIOB
- But if chip absent, reads return garbage/timeout
- Check `ESP_LOGI(TAG_MCP, "MCP23017 GPIOB ready")` in boot log

### Debug output expected:
With external pull-ups: idle = 0xFF on AW9523, 0x03 on MCP GPIOB (GPB0-1 HIGH, GPIOB2 LOW for LED off)
Pressed button: specific bit drops to 0, raw map shows corresponding bit

## Files to change
1. `ui_main.c` — fix R/D label font
2. `aw9523.c` — remove GCR + CTL writes
3. `buttons.c` — comprehensive per-read logging
4. `ui_logic.c` — keep edge logging (already there)
5. `mcp23017.c` — add GPIOB read log in set_led or when read
