# ZX7D00CE01S — Smart Serial LCD Display

## Core

| Param | Value |
|-------|-------|
| Module | WT32-S3-WROVER-N16R (ESP32-S3) |
| Flash | 16 MB |
| PSRAM | 8 MB (Octal) |
| Display | 7" TN, 800×480, RGB interface |
| Touch | Capacitive, I2C, no interrupt, reset via AW9523 P11 |
| Backlight | GPIO45 (PWM) |
| IO Expander | AW9523B on I2C bus |
| I2C Bus | GPIO47 (SCL), GPIO48 (SDA) — shared: touch + AW9523B |

## LCD RGB Interface

| Signal | GPIO | Signal | GPIO |
|--------|------|--------|------|
| D0 | 17 | D8 | 3 |
| D1 | 16 | D9 | 8 |
| D2 | 15 | D10 | 18 |
| D3 | 7 | D11 | 10 |
| D4 | 6 | D12 | 11 |
| D5 | 21 | D13 | 12 |
| D6 | 0 | D14 | 13 |
| D7 | 46 | D15 | 14 |
| PCLK | 9 | HSYNC | 5 |
| VSYNC | 38 | DE | 39 |
| RST | AW9523 P10 | BL_PWM | GPIO45 |

## Touch Interface

| Signal | Pin |
|--------|-----|
| TP_SCL | GPIO47 (shared I2C) |
| TP_SDA | GPIO48 (shared I2C) |
| TP_INT | NC |
| TP_RST | AW9523 P11 |

## Debug / Programming (7-pin Molex MX1.25)

| Pin | Signal | Range |
|-----|--------|-------|
| 1 | +5V | 5V |
| 2 | +3.3V | 3.3V (reference only) |
| 3 | ESP_TXD | TXD0, 3.3V TTL |
| 4 | ESP_RXD | RXD0, 3.3V TTL |
| 5 | EN | 0–3.3V |
| 6 | BOOT (GPIO0) | 0–3.3V |
| 7 | GND | 0V |

## Extended IO — 2.0mm 2×20P

### ESP32-S3 Direct GPIOs

| Pin | Signal | GPIO | Usage Plan |
|-----|--------|------|------------|
| 23 | EXT_GPIO_0 | 20 | Scale A (PCNT) |
| 24 | EXT_GPIO_1 | 19 | Scale B (PCNT) |
| 25 | EXT_GPIO_2 | 41 | Encoder A |
| 26 | EXT_GPIO_3 | 40 | Encoder B |
| 27 | EXT_GPIO_4 | 4 | Encoder button |
| 28 | EXT_GPIO_5 | 42 | D/R toggle button |
| 29 | EXT_GPIO_6 | 1 | Spare button |
| 30 | EXT_GPIO_7 | 2 | Free |
| 31 | I2C_SCL | 47 | Shared I2C bus |
| 32 | I2C_SDA | 48 | Shared I2C bus |

### AW9523B Expander GPIOs (3.3V TTL)

| Pin | Signal | AW9523 | Usage Plan |
|-----|--------|--------|------------|
| 11 | EXT_LSIO_0 | P0 | Radial T1 button |
| 12 | EXT_LSIO_1 | P1 | Radial T2 button |
| 13 | EXT_LSIO_2 | P2 | Radial T3 button |
| 14 | EXT_LSIO_3 | P3 | Radial T4 button |
| 15 | EXT_LSIO_4 | P4 | Axial T1 button |
| 16 | EXT_LSIO_5 | P5 | Axial T2 button |
| 17 | EXT_LSIO_6 | P6 | Axial T3 button |
| 18 | EXT_LSIO_7 | P7 | Axial T4 button |

### Power Pins

| Pins | Signal |
|------|--------|
| 1,2,39,40 | +5V input |
| 3,4,37,38 | +3.3V output (ref only) |
| 5–8,33–36 | GND |

## AW9523B Internal Assignments

| AW9523 | Function |
|--------|----------|
| P0–P7 | User buttons (EXT_LSIO_0–7) |
| P10 | LCD_RST |
| P11 | TP_RST |

## Flash Config

| Param | Value |
|-------|-------|
| Flash Size | 32 Mbit (4 MB) |
| SPI Mode | DIO |
| Crystal | 40 MHz |
