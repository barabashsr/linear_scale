# Font Generation Guide for LVGL 9.x

This guide explains how to generate bitmap fonts with Cyrillic support for the Welding Positioner UI.

## Prerequisites

Install `lv_font_conv` globally:

```bash
npm install -g lv_font_conv
```

## Font Files

Source TTF files are stored in this directory:
- `Roboto-Regular.ttf` - Regular weight
- `Roboto-Bold.ttf` - Bold weight

These were extracted from `roboto.zip` (Google Fonts official release).

## Unicode Ranges

| Range | Description |
|-------|-------------|
| `0x20-0x7F` | Basic Latin (ASCII printable characters) |
| `0x390-0x3C9` | Greek alphabet (Α-Ω, α-ω) - includes Ω for Ohm |
| `0x410-0x44F` | Cyrillic uppercase (А-Я) and lowercase (а-я) |
| `0x401` | Ё (Cyrillic capital letter Io) |
| `0x451` | ё (Cyrillic small letter io) |

## Generation Commands

### Regular Fonts

```bash
# 20px - Labels, info text
lv_font_conv --font Roboto-Regular.ttf \
  --bpp 4 --size 20 --no-compress \
  --range 0x20-0x7F \
  --range 0x390-0x3C9 \
  --range 0x410-0x44F \
  --range 0x401,0x451 \
  --format lvgl \
  --output ui_font_roboto_20.c \
  --lv-font-name ui_font_roboto_20

# 24px - Section titles
lv_font_conv --font Roboto-Regular.ttf \
  --bpp 4 --size 24 --no-compress \
  --range 0x20-0x7F \
  --range 0x390-0x3C9 \
  --range 0x410-0x44F \
  --range 0x401,0x451 \
  --format lvgl \
  --output ui_font_roboto_24.c \
  --lv-font-name ui_font_roboto_24

# 28px - Screen headers
lv_font_conv --font Roboto-Regular.ttf \
  --bpp 4 --size 28 --no-compress \
  --range 0x20-0x7F \
  --range 0x390-0x3C9 \
  --range 0x410-0x44F \
  --range 0x401,0x451 \
  --format lvgl \
  --output ui_font_roboto_28.c \
  --lv-font-name ui_font_roboto_28
```

### Bold Fonts

```bash
# 36px Bold - Parameter values
lv_font_conv --font Roboto-Bold.ttf \
  --bpp 4 --size 36 --no-compress \
  --range 0x20-0x7F \
  --range 0x390-0x3C9 \
  --range 0x410-0x44F \
  --range 0x401,0x451 \
  --format lvgl \
  --output ui_font_roboto_bold_36.c \
  --lv-font-name ui_font_roboto_bold_36

# 48px Bold - Large values, numpad
lv_font_conv --font Roboto-Bold.ttf \
  --bpp 4 --size 48 --no-compress \
  --range 0x20-0x7F \
  --range 0x390-0x3C9 \
  --range 0x410-0x44F \
  --range 0x401,0x451 \
  --format lvgl \
  --output ui_font_roboto_bold_48.c \
  --lv-font-name ui_font_roboto_bold_48
```

## Post-Generation: Fix Include Path

The generated files use `#include "lvgl/lvgl.h"` but ESP-IDF component uses `#include "lvgl.h"`. Fix with:

```bash
for f in ui_font_roboto_*.c; do
  sed -i '' 's|#include "lvgl/lvgl.h"|#include "lvgl.h"|g' "$f"
done
```

On Linux (without macOS sed):
```bash
for f in ui_font_roboto_*.c; do
  sed -i 's|#include "lvgl/lvgl.h"|#include "lvgl.h"|g' "$f"
done
```

## One-Liner Script

Generate all fonts at once:

```bash
cd main/ui/fonts

# Generate all fonts (Latin + Greek + Cyrillic)
lv_font_conv --font Roboto-Regular.ttf --bpp 4 --size 20 --no-compress --range 0x20-0x7F --range 0x390-0x3C9 --range 0x410-0x44F --range 0x401,0x451 --format lvgl --output ui_font_roboto_20.c --lv-font-name ui_font_roboto_20 && \
lv_font_conv --font Roboto-Regular.ttf --bpp 4 --size 24 --no-compress --range 0x20-0x7F --range 0x390-0x3C9 --range 0x410-0x44F --range 0x401,0x451 --format lvgl --output ui_font_roboto_24.c --lv-font-name ui_font_roboto_24 && \
lv_font_conv --font Roboto-Regular.ttf --bpp 4 --size 28 --no-compress --range 0x20-0x7F --range 0x390-0x3C9 --range 0x410-0x44F --range 0x401,0x451 --format lvgl --output ui_font_roboto_28.c --lv-font-name ui_font_roboto_28 && \
lv_font_conv --font Roboto-Bold.ttf --bpp 4 --size 36 --no-compress --range 0x20-0x7F --range 0x390-0x3C9 --range 0x410-0x44F --range 0x401,0x451 --format lvgl --output ui_font_roboto_bold_36.c --lv-font-name ui_font_roboto_bold_36 && \
lv_font_conv --font Roboto-Bold.ttf --bpp 4 --size 48 --no-compress --range 0x20-0x7F --range 0x390-0x3C9 --range 0x410-0x44F --range 0x401,0x451 --format lvgl --output ui_font_roboto_bold_48.c --lv-font-name ui_font_roboto_bold_48 && \
for f in ui_font_roboto_*.c; do sed -i '' 's|#include "lvgl/lvgl.h"|#include "lvgl.h"|g' "$f"; done && \
echo "All fonts generated successfully"
```

## Key Options Explained

| Option | Value | Description |
|--------|-------|-------------|
| `--bpp` | 4 | Bits per pixel (4 = 16 levels of anti-aliasing) |
| `--no-compress` | - | **Required for LVGL 9.x** - Disables RLE compression |
| `--format` | lvgl | Output C file for LVGL |
| `--range` | hex | Unicode range to include |

## Important Notes

1. **Always use `--no-compress`** - Compressed fonts cause rendering artifacts in LVGL 9.x
2. **Use correct TTF files** - Download from Google Fonts official releases
3. **Fix includes after generation** - ESP-IDF uses different include path
4. **Update CMakeLists.txt** - If adding/removing fonts, update `main/CMakeLists.txt`

## Font Aliases (ui_fonts.h)

```c
#define UI_FONT_LABEL       &ui_font_roboto_20      /* Labels, info text */
#define UI_FONT_TITLE       &ui_font_roboto_24      /* Section titles */
#define UI_FONT_HEADER      &ui_font_roboto_28      /* Screen headers */
#define UI_FONT_VALUE       &ui_font_roboto_bold_36 /* Parameter values */
#define UI_FONT_LARGE       &ui_font_roboto_bold_48 /* Large values, numpad */
```

## Adding New Font Sizes

1. Generate the font with `lv_font_conv`
2. Fix the include path
3. Add declaration to `ui_fonts.h`:
   ```c
   LV_FONT_DECLARE(ui_font_roboto_XX);
   ```
4. Add to `main/CMakeLists.txt`:
   ```cmake
   set(FONT_SOURCES
       ...
       ui/fonts/ui_font_roboto_XX.c
   )
   ```

## Troubleshooting

### Corrupted/garbled Cyrillic characters
- Ensure `--no-compress` flag is used
- Verify TTF file is not corrupted (re-extract from zip)

### "Cannot load font" error
- Check TTF file exists and is valid
- Try re-downloading the font

### Missing characters
- Verify Unicode ranges include all needed characters
- For Ukrainian, add: `--range 0x404,0x406,0x407,0x454,0x456,0x457` (Є, І, Ї, є, і, ї)
