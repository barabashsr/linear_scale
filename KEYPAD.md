# 4×4 Matrix Keypad via MCP23017 I2C Port Expander

## 1. Matrix Keypad Operation Principle

### 1.1 Physical Layout

A 4×4 membrane keypad contains 16 tactile switches arranged at intersections of 4 row wires and 4 column wires:

```
     C0   C1   C2   C3
      |    |    |    |
R0 ---+----+----+----+---   [ 1 ] [ 2 ] [ 3 ] [ A ]
R1 ---+----+----+----+---   [ 4 ] [ 5 ] [ 6 ] [ B ]
R2 ---+----+----+----+---   [ 7 ] [ 8 ] [ 9 ] [ C ]
R3 ---+----+----+----+---   [ * ] [ 0 ] [ # ] [ D ]
```

Physical keypad face:

```
┌─────┬─────┬─────┬─────┐
│  1  │  2  │  3  │  A  │
├─────┼─────┼─────┼─────┤
│  4  │  5  │  6  │  B  │
├─────┼─────┼─────┼─────┤
│  7  │  8  │  9  │  C  │
├─────┼─────┼─────┼─────┤
│  *  │  0  │  #  │  D  │
└─────┴─────┴─────┴─────┘
      4×4 Matrix Membrane Keypad
      ~70×70 mm, ~10 mm thick
             8-pin FPC or pin header tail
```

Keypad — UI Action Map (for on-screen representation):

```
┌─────┬─────┬─────┬──────────┐
│  1  │  2  │  3  │    OK    │
├─────┼─────┼─────┼──────────┤
│  4  │  5  │  6  │  Cancel  │
├─────┼─────┼─────┼──────────┤
│  7  │  8  │  9  │  Clear   │
├─────┼─────┼─────┼──────────┤
│  .  │  0  │  ±  │    ⌫    │
└─────┴─────┴─────┴──────────┘
```

Each key connects the corresponding row line to column line when pressed. No diodes are required — the matrix relies on the one-directional scanning pattern (only one row driven HIGH at a time, all others LOW).

### 1.2 Scanning Algorithm

1. **Idle state**: All 4 rows driven LOW (output logic 0). All 4 columns configured as inputs with internal pull-up resistors enabled — columns read HIGH when no key is pressed.

2. **Scan pass** (4 steps, one per row):
   - Set row `i` **HIGH** (output 1), keep all other rows LOW
   - Read the 4 column inputs
   - Column `j` reading **LOW** → key `(i, j)` is currently pressed
   - Column `j` reading **HIGH** → key `(i, j)` is released
   - Restore row `i` to LOW, move to next row

3. **Why drive HIGH instead of LOW for active row?**

   Because all 4 rows are LOW in idle. If two keys on the same column are pressed simultaneously, they would short a HIGH row to a LOW row through the column wire, causing an electrical conflict (phantom keypress / ghosting). Driving only one row HIGH at a time eliminates this — at most one row can pull a column LOW through a pressed key, and the other pressed key on the same column simply pulls the column to LOW via the LOW row (which is harmless and detected as "not pressed" since its row is not the active one).

   **Critical**: Never drive more than one row HIGH simultaneously during scanning. Each scan step is:
   ```
   Row output byte = (1 << i)    // only bit i is 1 (driven HIGH)
   ```
   Not a bitmask inversion. All other rows remain LOW.

4. **Debounce**: A complete scan produces a 16-bit bitmap of pressed keys. This raw bitmap must be compared across 3 consecutive scan passes (30 ms total at 10 ms interval). Only keys that appear in all 3 consecutive passes are considered "stable" and eligible for reporting. This is identical to the existing debounce pattern in `buttons.c:35–47`.

5. **Edge detection**: The stable bitmap is compared to the previous stable bitmap. A key transitioning from 0→1 (newly pressed) triggers a character event. A key held down does not generate repeat events (no auto-repeat). This prevents unintended duplicate inputs from long presses.

---

## 2. MCP23017 I2C Port Expander

### 2.1 Overview

The MCP23017 is a 16-bit I2C GPIO expander with two 8-bit ports (GPIOA and GPIOB). Key features:

- I2C interface up to 1.7 MHz (400 kHz used in this project)
- 16 individually configurable GPIO pins
- Internal pull-up resistors (100 kΩ, configurable per pin)
- Interrupt-on-change capability (per pin, not used in this design)
- 3 address select pins (A0, A1, A2) → 8 possible I2C addresses

### 2.2 I2C Address Configuration

| Pin | Connection | Value |
|-----|-----------|-------|
| A0  | GND       | 0     |
| A1  | GND       | 0     |
| A2  | GND       | 0     |

Base address: `0b0100 000 = 0x20`

**Address conflict check**: The AW9523B already on the same I2C bus uses address `0x5B`. These do not conflict.

### 2.3 Pin Assignment

| MCP23017 Pin | Name   | Connected To         | Direction | Role       |
|-------------|--------|----------------------|-----------|------------|
| 21          | GPA0   | Keypad Column 0 (C0) | Input     | Column 0   |
| 22          | GPA1   | Keypad Column 1 (C1) | Input     | Column 1   |
| 23          | GPA2   | Keypad Column 2 (C2) | Input     | Column 2   |
| 24          | GPA3   | Keypad Column 3 (C3) | Input     | Column 3   |
| 25          | GPA4   | —                    | —         | Unused     |
| 26          | GPA5   | —                    | —         | Unused     |
| 27          | GPA6   | —                    | —         | Unused     |
| 28          | GPA7   | —                    | —         | Unused     |
| 1           | GPB0   | Keypad Row 0 (R0)    | Output    | Row 0      |
| 2           | GPB1   | Keypad Row 1 (R1)    | Output    | Row 1      |
| 3           | GPB2   | Keypad Row 2 (R2)    | Output    | Row 2      |
| 4           | GPB3   | Keypad Row 3 (R3)    | Output    | Row 3      |
| 5           | GPB4   | —                    | —         | Unused     |
| 6           | GPB5   | —                    | —         | Unused     |
| 7           | GPB6   | —                    | —         | Unused     |
| 8           | GPB7   | —                    | —         | Unused     |
| 12          | SCL    | ESP32-S3 GPIO 47     | —         | I2C Clock  |
| 13          | SDA    | ESP32-S3 GPIO 48     | —         | I2C Data   |
| 9           | VDD    | +3.3V                | —         | Power      |
| 10          | VSS    | GND                  | —         | Ground     |
| 18          | RESET  | +3.3V (via 10kΩ)     | —         | Held HIGH  |
| 20          | INTA   | — (unused)           | —         | Floating   |
| 19          | INTB   | — (unused)           | —         | Floating   |

**Design rationale for port separation (GPIOA=columns, GPIOB=rows)**:
- Separate I/O direction per port: GPIOA is entirely inputs, GPIOB is entirely outputs
- No per-pin direction mixing within a port — cleaner register setup
- Column reads from GPIOA register (0x12) do not affect GPIOB output state
- Row writes to GPIOB/OLATB register (0x13/0x15) are independent

### 2.4 Register Map (BANK=0)

| Address | Name      | Read/Write | Description                        |
|---------|-----------|------------|------------------------------------|
| 0x00    | IODIRA    | R/W        | I/O direction: GPIOA (1=input)     |
| 0x01    | IODIRB    | R/W        | I/O direction: GPIOB (1=input)     |
| 0x02    | IPOLA     | R/W        | Input polarity: GPIOA              |
| 0x03    | IPOLB     | R/W        | Input polarity: GPIOB              |
| 0x04    | GPINTENA  | R/W        | Interrupt-on-change: GPIOA         |
| 0x05    | GPINTENB  | R/W        | Interrupt-on-change: GPIOB         |
| 0x06    | DEFVALA   | R/W        | Default compare value: GPIOA       |
| 0x07    | DEFVALB   | R/W        | Default compare value: GPIOB       |
| 0x08    | INTCONA   | R/W        | Interrupt control: GPIOA           |
| 0x09    | INTCONB   | R/W        | Interrupt control: GPIOB           |
| 0x0A    | IOCONA    | R/W        | Configuration: GPIOA               |
| 0x0B    | IOCONB    | R/W        | Configuration: GPIOB               |
| 0x0C    | GPPUA     | R/W        | Pull-up resistor: GPIOA            |
| 0x0D    | GPPUB     | R/W        | Pull-up resistor: GPIOB            |
| 0x0E    | INTFA     | RO         | Interrupt flag: GPIOA              |
| 0x0F    | INTFB     | RO         | Interrupt flag: GPIOB              |
| 0x10    | INTCAPA   | RO         | Interrupt capture: GPIOA           |
| 0x11    | INTCAPB   | RO         | Interrupt capture: GPIOB           |
| 0x12    | GPIOA     | R/W        | Port value: GPIOA                  |
| 0x13    | GPIOB     | R/W        | Port value: GPIOB                  |
| 0x14    | OLATA     | R/W        | Output latch: GPIOA                |
| 0x15    | OLATB     | R/W        | Output latch: GPIOB                |

### 2.5 Initialization Sequence

At startup (`mcp23017_init()`):

```
Step 1: IOCONA ← 0x00
        - BANK=0 (registers in paired layout as shown above)
        - SEQOP=0 (sequential addressing enabled — multi-byte writes auto-increment)
        - MIRROR=0 (INTA and INTB independent — not used but set for clarity)

Step 2: IODIRA ← 0xFF     All GPIOA pins = inputs
        IODIRB ← 0x00     All GPIOB pins = outputs

Step 3: GPPUA  ← 0x0F     Pull-ups on GPA0-GPA3 (columns only)
        GPPUB  ← 0x00     No pull-ups on GPIOB (outputs don't need them)

Step 4: IPOLA  ← 0x00     No polarity inversion
        IPOLB  ← 0x00

Step 5: GPINTENA ← 0x00   No interrupt-on-change (timer-based scanning)
        GPINTENB ← 0x00

Step 6: OLATB  ← 0x00     All rows LOW (idle state)
```

**Important**: Port pins have undefined state after power-up. The MCP23017 must be explicitly configured before any GPIO interaction. On ZX7D00CE01S, the I2C bus is already initialized in `main.c:77–83` before `keypad_init()` is called.

### 2.6 I2C Communication Details

All MCP23017 I2C transactions use the existing I2C bus (I2C_NUM_0, SCL=GPIO 47, SDA=GPIO 48, 400 kHz).

**Write register**: Send device address + W, register address, data byte(s).
```
START | 0x40 | ACK | reg_addr | ACK | data | ACK | STOP
```

**Read register**: Send device address + W, register address, then repeated START + device address + R, read data byte(s).
```
START | 0x40 | ACK | reg_addr | ACK | RESTART | 0x41 | ACK | data | NAK | STOP
```

ESP-IDF I2C API used:
- `i2c_master_write_to_device()` — write with auto START/STOP
- `i2c_master_write_read_device()` — combined write-then-read

**Timeout**: 10 ms for keypad operations.

---

## 3. Hardware Wiring

### 3.1 ZX7D00CE01S Extended IO Connector

The Panlee board exposes I2C and power on the 40-pin Extended IO Interface:

| Connector Pin | Signal     | Connect To       |
|---------------|------------|------------------|
| Pin 1         | +5V        | —                |
| Pin 3         | +3.3V      | MCP23017 VDD     |
| Pin 5         | GND        | MCP23017 VSS     |
| Pin 31        | IIC_SCL    | MCP23017 SCL     |
| Pin 32        | IIC_SDA    | MCP23017 SDA     |

### 3.2 MCP23017 to Keypad

| MCP23017 | Direction | Keypad Connector Pin (typical) |
|----------|-----------|-------------------------------|
| GPB0     | Output    | Row 0 (R0)                    |
| GPB1     | Output    | Row 1 (R1)                    |
| GPB2     | Output    | Row 2 (R2)                    |
| GPB3     | Output    | Row 3 (R3)                    |
| GPA0     | Input     | Column 0 (C0)                 |
| GPA1     | Input     | Column 1 (C1)                 |
| GPA2     | Input     | Column 2 (C2)                 |
| GPA3     | Input     | Column 3 (C3)                 |

### 3.3 BOM

| Part              | Qty | Notes                                    |
|-------------------|-----|------------------------------------------|
| MCP23017-E/SP     | 1   | DIP-28 or SOIC-28 with adapter           |
| 4×4 Matrix Keypad | 1   | Standard 8-pin membrane type             |
| 10 kΩ resistor    | 1   | Pull-up on RESET pin                     |
| 100 nF capacitor  | 1   | Decoupling across VDD-VSS (recommended)  |
| Jumper wires      | 8   | MCP23017 ↔ keypad                        |
| Jumper wires      | 4   | ZX7D00CE01S ↔ MCP23017 (VCC, GND, SDA, SCL) |

---

## 4. Software Architecture

### 4.1 Module Dependencies

```
main.c
  ├── keypad.h          (new)
  │     ├── mcp23017.h  (new)
  │     └── (esp_timer)
  └── ui_main.h         (modified)
```

`keypad.c` depends on `mcp23017.c` for I2C register I/O. `main.c` depends on `keypad.c` for polling. `ui_main.c` depends on `keypad.h` for enable/disable control.

### 4.2 mcp23017.h API

```c
#pragma once
#include "esp_err.h"
#include <stdint.h>

#define MCP23017_ADDR_BASE  0x20

esp_err_t mcp23017_init(uint8_t addr);
esp_err_t mcp23017_write(uint8_t addr, uint8_t reg, uint8_t val);
esp_err_t mcp23017_read(uint8_t addr, uint8_t reg, uint8_t *val);
```

- `mcp23017_init(addr)` — probe the device (read IODIRA, expect ACK), configure GPIOA/B directions, enable pull-ups on GPA0-3, set all rows LOW
- `mcp23017_write(addr, reg, val)` — single-byte I2C write to register
- `mcp23017_read(addr, reg, *val)` — single-byte I2C read from register

### 4.3 keypad.h API

```c
#pragma once
#include "esp_err.h"
#include <stdbool.h>

esp_err_t keypad_init(void);
void     keypad_enable(bool enable);
bool     keypad_get_char(char *c);
```

- `keypad_init()` — initializes MCP23017, creates 10 ms polling timer (stopped initially)
- `keypad_enable(true)` — starts the polling timer
- `keypad_enable(false)` — stops the polling timer, resets debounce state
- `keypad_get_char(*c)` — returns `true` and writes character to `*c` if a new key press was detected since last call. Returns `false` if no new key. Call from main loop at ~1 ms rate.

### 4.4 keypad.c Internal Logic

#### State Machine

```
                    +----------+
                    | DISABLED |   timer stopped, no I2C traffic
                    +----------+
                          |
               keypad_enable(true)
                          |
                          v
                    +----------+
                    | SCANNING |   timer fires every 10 ms
                    +----------+
                          |
               keypad_enable(false)
                          |
                          v
                    +----------+
                    | DISABLED |   timer stopped, state reset
                    +----------+
```

#### Data Structures

```c
#define KP_ROWS          4
#define KP_COLS          4
#define KP_DEBOUNCE_MS   30      // 3 ticks × 10 ms
#define KP_POLL_MS       10

static esp_timer_handle_t kp_timer;
static volatile uint16_t  kp_stable;     // debounced key bitmap (16 keys)
static volatile uint16_t  kp_reported;   // already-reported keys (for edge detection)
static volatile bool      kp_char_ready; // new character available
static volatile char      kp_char;       // the new character
```

#### Timer Callback (kp_timer_cb)

```
1. Read raw key bitmap:
   raw = 0
   for row = 0 to 3:
       mcp23017_write(ADDR, OLATB, 1 << row)     // drive one row HIGH
       delay_us(10)                                // I2C + pin settling
       cols = mcp23017_read(ADDR, GPIOA) & 0x0F   // read column inputs
       for col = 0 to 3:
           if (cols & (1 << col)) == 0:            // column LOW = key pressed
               raw |= (1 << (row * 4 + col))
   mcp23017_write(ADDR, OLATB, 0x00)              // restore: all rows LOW

2. Debounce:
   static uint16_t prev_raw = 0, candidate = 0
   static int stable_cnt = 0
   if raw == prev_raw:
       if ++stable_cnt >= 3:
           candidate = raw
   else:
       stable_cnt = 0
   prev_raw = raw

3. Edge detection & character lookup:
   uint16_t new_keys = candidate & ~kp_reported
   if new_keys:
       find first set bit in new_keys
       lookup character from key map table
       set kp_char, kp_char_ready = true
   kp_reported = candidate
```

#### Key Map Table

```c
static const char kp_map[KP_ROWS][KP_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
};
```

Row 0 = GPB0 drives HIGH, Row 1 = GPB1, etc.
Column 0 = GPA0 reads, Column 1 = GPA1, etc.

#### keypad_get_char Implementation

```c
bool keypad_get_char(char *c)
{
    if (!kp_char_ready) return false;
    *c = kp_char;
    kp_char_ready = false;
    return true;
}
```

---

## 5. Integration with Existing Code

### 5.1 config.h Additions

```c
/* ── MCP23017 keypad expander ────────────────── */
#define CFG_MCP23017_ADDR       0x20
#define CFG_MCP23017_SDA        CFG_I2C_SDA    // GPIO 48 (shared I2C bus)
#define CFG_MCP23017_SCL        CFG_I2C_SCL    // GPIO 47 (shared I2C bus)
#define CFG_KP_POLL_MS          10
#define CFG_KP_DEBOUNCE_TICKS   3
```

### 5.2 CMakeLists.txt Changes

Add to `SRCS`:
```
"mcp23017.c"
"keypad.c"
```

### 5.3 main.c Changes

```c
// After buttons_init() [currently line 96]:
ESP_ERROR_CHECK(keypad_init());

// In the main while(1) loop [after line 132]:
char kp_char;
if (keypad_get_char(&kp_char)) {
    ui_main_handle_keypad(kp_char);
}
```

**Thread safety note**: `keypad_get_char()` reads `volatile` variables set by the timer callback. The FreeRTOS task and timer callback may run concurrently. Because only single bytes/booleans are read/written atomically on ESP32 and because only the timer writes and only the main loop reads, this is safe without a mutex.

### 5.4 ui_main.h Addition

```c
void ui_main_handle_keypad(char c);
```

### 5.5 ui_main.c Changes

#### show_numpad() — Enable Keypad

At the start of `show_numpad()`, add:
```c
keypad_enable(true);
```

#### Close Paths — Disable Keypad

In `np_btn_cb()` (line 69), when `"OK"` is pressed:
```c
if (strcmp(c, "OK") == 0) {
    keypad_enable(false);
    // ... existing OK logic ...
}
```

In the keypad handler (below), cancel (`"B"`) and OK (`"A"`) also call `keypad_enable(false)`.

#### ui_main_handle_keypad Implementation

Defined in `ui_main.c` (has access to all `static` symbols):

```c
void ui_main_handle_keypad(char c)
{
    switch (c) {
    case 'A':   // OK — confirm value, close modal
    {
        keypad_enable(false);
        float v = atof(g_numpad_buf);
        logic_set_diameter(v);
        if (g_modal) { lv_obj_del(g_modal); g_modal = NULL; g_np_label = NULL; }
        break;
    }
    case 'B':   // Cancel — close modal without saving
        keypad_enable(false);
        if (g_modal) { lv_obj_del(g_modal); g_modal = NULL; g_np_label = NULL; }
        break;
    case 'C':   // Clear — reset buffer to empty
        g_numpad_buf[0] = '\0';
        np_update_label();
        logic_set_diameter(0.0f);
        break;
    case 'D':   // Backspace — delete last character
        np_backspace();
        np_update_label();
        logic_set_diameter(atof(g_numpad_buf));
        break;
    case '*':   // Decimal point
        np_add_char('.');
        np_update_label();
        logic_set_diameter(atof(g_numpad_buf));
        break;
    case '#':   // Toggle sign +/−
        np_toggle_sign();
        np_update_label();
        logic_set_diameter(atof(g_numpad_buf));
        break;
    default:    // Digits 0–9
        if (c >= '0' && c <= '9') {
            np_add_char(c);
            np_update_label();
            logic_set_diameter(atof(g_numpad_buf));
        }
        break;
    }
}
```

**Note**: `ui_main_handle_keypad()` is defined in `ui_main.c` itself so it has direct access to `static` symbols (`np_add_char`, `np_backspace`, `np_toggle_sign`, `np_update_label`, `g_numpad_buf`, `g_modal`, `g_np_label`).

### 5.6 Key Mapping Summary

| Key | Raw Char | Action                              | Function Path             |
|-----|----------|-------------------------------------|---------------------------|
| 1–9 | `'1'`–`'9'` | Append digit to buffer           | `np_add_char(c)`         |
| 0   | `'0'`    | Append digit to buffer              | `np_add_char('0')`       |
| A   | `'A'`    | OK — confirm value, close modal     | `logic_set_diameter()` + delete modal |
| B   | `'B'`    | Cancel — close modal, discard       | delete modal              |
| C   | `'C'`    | Clear — reset entire buffer         | `g_numpad_buf[0] = '\0'` |
| D   | `'D'`    | Backspace — remove last char        | `np_backspace()`          |
| *   | `'*'`    | Decimal point                       | `np_add_char('.')`       |
| #   | `'#'`    | Toggle sign (+/−)                   | `np_toggle_sign()`        |

---

## 6. Timing Analysis

### 6.1 I2C Bus Contention

The I2C bus (I2C_NUM_0, 400 kHz) is shared by:

| Device       | Address | Access Pattern                         | Transaction Time |
|-------------|---------|----------------------------------------|-----------------|
| GT911 Touch | 0x5D    | Continuous (~100 Hz touch polling)     | ~0.2 ms each    |
| AW9523      | 0x5B    | Every 1 ms (buttons_read via AW9523)  | ~0.1 ms each    |
| CH422G BL   | 0x38/0x24 | Only at boot                           | ~0.1 ms         |
| MCP23017    | 0x20    | Every 10 ms when numpad is open        | ~0.3 ms per scan |

**Keypad scan I2C transaction budget per 10 ms tick**:
- 4 × (write OLATB: 3 bytes) = 12 bytes written
- 4 × (read GPIOA: 2 + 1 bytes) = 12 bytes read
- Total: ~24 bytes @ 400 kHz ≈ **0.5 ms** per scan pass
- 10 ms period → ~5% I2C bus utilization during numpad-open

**No meaningful I2C contention** — the touch controller uses LVGL's I2C lock, and the 1 ms main loop is a cooperative task. If the keypad timer fires during a `buttons_read()` transaction, ESP-IDF's I2C driver queues the second transaction (~0.1 ms delay on the timer callback, acceptable for 10 ms period).

### 6.2 Scan Latency

- Timer fires → 0.5 ms I2C scan → debounce logic → character ready
- Worst case: 10 ms (timer period) + 0.5 ms (scan) = **10.5 ms** until character appears in `keypad_get_char()`
- Main loop polls at 1 ms → worst case additional 1 ms before `ui_main_handle_keypad()` is called
- **Total worst-case latency: ~12 ms** — imperceptible to the user

### 6.3 Debounce Window

- 3 consecutive matching scans × 10 ms = **30 ms stable window**
- Mechanical key bounce typically settles within 5–15 ms
- 30 ms is conservative and ensures no double-triggers

---

## 7. Error Handling

### 7.1 MCP23017 Not Found

If `mcp23017_init()` fails the I2C probe (no ACK at address 0x20):
- Log warning: `"MCP23017 not found at 0x20 — keypad disabled"`
- Return `ESP_ERR_NOT_FOUND`
- `keypad_init()` must **not** prevent the DRO from booting — the system remains functional without the keypad
- `keypad_enable()` becomes a no-op when init failed
- `keypad_get_char()` always returns `false`

### 7.2 I2C Errors During Operation

If an I2C read/write fails during scanning:
- Log the error once (not every scan, to avoid log spam)
- Skip the current scan frame (raw bitmap = 0, which resets debounce counter)
- On next scan, if I2C recovers, debounce starts fresh — safe fallback

### 7.3 MCP23017 Reset / Power Glitch

If the MCP23017 resets or loses power:
- Ports revert to inputs (high impedance)
- Keypad scan will report all-zero (no keys pressed)
- Keypad effectively "stops working" but does not crash the system
- MCP23017 configuration is restored on next full system boot

---

## 8. Test Checklist

1. **I2C probe**: Verify MCP23017 responds at 0x20
2. **Idle state**: After `mcp23017_init()`, read GPIOA → expect 0x0F (all columns HIGH due to pull-ups, no keys pressed)
3. **Single key**: Press key `1` (row 0, col 0) → read OLATB=0x01 (row 0 HIGH), read GPIOA → bit 0 should be LOW
4. **All keys**: Press each of 16 keys, verify correct character reported
5. **Multiple keys**: Press 2 keys on same column → verify correct detection (no ghosting)
6. **Debounce**: Rapid tap → verify only one character reported per press
7. **Enable/disable**: Open numpad → verify timer runs. Close numpad → verify timer stops, no I2C traffic
8. **No keypad**: Boot without MCP23017 connected → verify DRO functions normally (no crash)

---

## 9. Source File Summary

| File | Lines (est.) | Key Contents |
|------|-------------|-------------|
| `mcp23017.h` | ~15 | Register `#define`s, function prototypes |
| `mcp23017.c` | ~80 | `mcp23017_init()`: I2C probe, direction, pull-up config. `mcp23017_write()`/`mcp23017_read()`: 1-byte I2C wrappers |
| `keypad.h` | ~10 | `keypad_init()`, `keypad_enable()`, `keypad_get_char()` declarations |
| `keypad.c` | ~130 | Timer setup, scan routine, debounce state machine, key map table, character output |
| `config.h` | +5 lines | MCP23017 address, poll interval |
| `main.c` | +5 lines | `keypad_init()` call, poll in main loop |
| `ui_main.h` | +1 line | `ui_main_handle_keypad()` declaration |
| `ui_main.c` | +40 lines | Enable/disable calls in numpad open/close, handler function |
| `CMakeLists.txt` | +2 lines | Add source files |

**Total: ~290 lines of new code, ~50 lines modified.**

---

## 10. Future Considerations

### 10.1 GPIOA4–A7, GPIOB4–B7 Expansion

8 spare MCP23017 pins are available. Possibilities:
- 4 additional direct GPIO buttons for DRO functions (zero, D/R toggle)
- Rotary encoder quadrature input (2 pins)
- LED indicators for status

### 10.2 Interrupt-Driven Alternative

If timer-based scanning proves insufficient (unlikely given 10 ms polling), the MCP23017 GPINTEN + INTA pin can be used:
- Configure GPINTENA for all 4 column pins
- Connect INTA to an ESP32 GPIO (e.g., GPIO 42)
- GPIO falling-edge ISR sets a flag; main loop performs a full scan only when flag is set
- Requires handling MCP23017 interrupt clearing (reading INTCAPA/GPIOA)

### 10.3 Numpad for Both Axes

Currently the numpad only sets radial axis diameter. Could be extended to accept offset values for the axial axis as well — toggle with a physical key (e.g., long-press `D`).
