# UI Layout Final Plan — 3-Column Layout

## Top-level 3-column layout
```
┌──────────────────────────────────────────────────────────────────────┐
│                             800×480                                   │
│                                                                       │
│  COL1            COL2                         COL3                   │
│  (left btns)     (axes plates)                (right btns)            │
│  x=4 w=64        x=72 w=660                    x=736 w=64             │
│                                                                       │
│  ┌──┐  ┌──────────────────────────────────┐  ┌──┐                    │
│  │⌀ │  │ y=17 РАДИАЛЬНАЯ (мм)             │  │ 0│   ← Radial (y=8)  │
│  ├──┤  │ y=51 +123.456                    │  ├──┤                   │
│  │D/│  │        ┌───────────┐ y=3         │  │ 0│                   │
│  │R │  │        │ +12.345   │  SP1        │  └──┘                   │
│  └──┘  │        └───────────┘             │                          │
│        │        ┌───────────┐ y=77        │                          │
│        │        │ +45.678   │  SP2        │                          │
│        │        └───────────┘             │                          │
│        └──────────────────────────────────┘                          │
│                                                                       │
│  ┌──┐  ┌──────────────────────────────────┐  ┌──┐                    │
│  │ 0│  │ y=17 ОСЕВАЯ (мм)                 │  │ 0│   ← Axial (y=162) │
│  └──┘  │ y=51 +78.900                     │  ├──┤                   │
│        │        ┌───────────┐ y=3         │  │ 0│                   │
│        │        │ +12.345   │             │  └──┘                   │
│        │        └───────────┘             │                          │
│        │        ┌───────────┐ y=77        │                          │
│        │        │ +45.678   │             │                          │
│        │        └───────────┘             │                          │
│        └──────────────────────────────────┘                          │
│                                                                       │
│        ┌──────────────────────────────────┐  ┌──┐                    │
│        │ y=17 ШПИНДЕЛЬ (°)                │  │ 0│   ← Spindle (316) │
│        │ y=51 123.4°                      │  └──┘                   │
│        │        ┌───────────┐ y=3          │                          │
│        │        │ 123.4°    │  angle       │                          │
│        │        └───────────┘              │                          │
│        │        ┌───────────┐ y=77         │                          │
│        │        │1500 ОБ/МИН│  RPM (no btn)│                          │
│        │        └───────────┘              │                          │
│        └──────────────────────────────────┘                          │
└──────────────────────────────────────────────────────────────────────┘
```

## Dimensions
- ROW_H=148, ROW_GAP=6, ROW_PAD=8
- COL1: x=4, w=64 (56px btn + 8px gap to plate)
- COL2: x=72, w=660 (row plates live here)
- COL3: x=736, w=64 (56px btn + 8px margin from screen right)
- Row Y positions: Radial=8, Axial=162, Spindle=316

## Setpoint plates (inside COL2 row plate)
- SP_W=230, SP_H=68, SP_GAP=6
- Local x = 660 - 8 - 230 = 422
- SP_Y1 = (148-142)/2 = 3, SP_Y2 = 77
- Font: UI_FONT_LARGE (60px Roboto Bold)
- Value CENTER-aligned inside plate: `LV_ALIGN_CENTER`
- Color: CFG_LV_SCARLET (#FF2400) when active, CFG_LV_TEXT_DIM when inactive

## Center content (title+value, vertically centered)
- Title y=17, Value y=51 (total: 28+6+80=114, top=(148-114)/2=17)
- Title: "РАДИАЛЬНАЯ (мм)", "ОСЕВАЯ (мм)", "ШПИНДЕЛЬ (°)"
- Value: "+123.456" (no unit suffix)

## Buttons (screen-level, children of `scr`)
- Left btns (COL1): screen x=4, y = row_y + btn_offset
  - Radial: ⌀ at offset=16, D/R at offset=76
  - Axial: 0 at offset=16 (top-aligned, same as ⌀)
  - Spindle: none
- Right btns (COL3): screen x=736, y = row_y + btn_offset
  - 0 buttons at offsets SP_Y1 and SP_Y2 (matching setpoint positions)
  - No button for spindle RPM

## Degree sign
- U+00B0 not in Roboto Cyrillic
- Separate Montserrat 24-28px label for ° positioned to the right of:
  - Spindle main value (next to "123.4")
  - Spindle angle setpoint (next to "123.4")
- Label uses `&lv_font_montserrat_24`

## Files to change
1. `config.h` — add `CFG_COLOR_SCARLET 0xFF2400` + `CFG_LV_SCARLET`
2. `ui_main.c` — 3-column layout, all adjustments above
