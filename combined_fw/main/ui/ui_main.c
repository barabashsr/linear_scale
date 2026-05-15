#include "ui_main.h"
#include "ui_logic.h"
#include "ui_styles.h"
#include "config.h"
#include "i2c_protocol.h"
#include "keypad.h"
#include "ui_fonts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SP_C     CFG_MAX_SETPOINTS
#define ROW_H    152
#define ROW_GAP  6
#define ROW_PAD  0

#define COL1_X   4
#define COL2_X   72
#define COL2_W   660
#define COL3_X   736

#define BTN_SQ   66
#define SP_H     BTN_SQ
#define SP_GAP   6

#define SP_LOCX  358
#define SP_W     (COL2_W - SP_LOCX - 4)
#define SP_Y1    7
#define SP_Y2    (SP_Y1 + SP_H + SP_GAP)

#define TITLE_Y  19
#define VALUE_Y  53
#define VAL_W    (SP_LOCX - 20)

typedef struct {
    lv_obj_t *row, *title, *val;
    lv_obj_t *rd_label;
    lv_obj_t *sp_p[SP_C], *sp_v[SP_C];
} ax_ui_t;

static ax_ui_t g_ax[2];

static lv_obj_t *g_sp_row, *g_sp_title, *g_sp_val;
static lv_obj_t *g_sp_ang_v, *g_sp_diam_v;

static lv_obj_t *g_dlg, *g_modal, *g_underlay;
static int g_dlg_axis;
static char g_numpad_buf[16];
static lv_obj_t *g_np_label;

static void fpos(char *b, int sz, float mm)
{
    if (mm > -0.0005f && mm < 0.0005f) snprintf(b, sz, "+0.000");
    else if (mm < 0) snprintf(b, sz, "-%.3f", (double)(-mm));
    else snprintf(b, sz, "+%.3f", (double)mm);
}

/* ── UNDERLAY ── */
static void underlay_close_cb(lv_event_t *e) {
    keypad_enable(false);
    if (g_modal) { lv_obj_del(g_modal); g_modal = NULL; }
    if (g_underlay) { lv_obj_del(g_underlay); g_underlay = NULL; }
    g_np_label = NULL;
}
static void underlay_dlg_close_cb(lv_event_t *e) {
    if (g_dlg) { lv_obj_del(g_dlg); g_dlg = NULL; }
    if (g_underlay) { lv_obj_del(g_underlay); g_underlay = NULL; }
}
static lv_obj_t *underlay_create(lv_event_cb_t cb) {
    lv_obj_t *u = lv_obj_create(lv_scr_act());
    lv_obj_set_size(u, 800, 480); lv_obj_set_pos(u, 0, 0);
    lv_obj_set_style_bg_color(u, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(u, LV_OPA_40, 0);
    lv_obj_set_style_border_width(u, 0, 0);
    lv_obj_add_flag(u, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(u, cb, LV_EVENT_CLICKED, NULL);
    return u;
}

/* ── NUMPAD MODAL ── */
static void np_set_display(void) {
    if (g_np_label) lv_label_set_text(g_np_label, g_numpad_buf);
}

static void np_place_digit(char c) {
    int len = strlen(g_numpad_buf);
    if (len == 2 && g_numpad_buf[1] == '0') {
        g_numpad_buf[1] = c;
    } else {
        if (!strchr(g_numpad_buf, '.') && len >= 4) {
            g_numpad_buf[len] = '.'; len++;
        }
        char *dot = strchr(g_numpad_buf, '.');
        if (dot && strlen(dot + 1) >= 3) {
            if (len < 15) { memmove(dot + 2, dot + 3, strlen(dot + 3) + 1); }
        }
        if (len < 15) { g_numpad_buf[len] = c; g_numpad_buf[len + 1] = 0; }
    }
    np_set_display();
}

static void np_backspace(void) {
    int len = strlen(g_numpad_buf);
    if (len <= 2) { g_numpad_buf[1] = '0'; g_numpad_buf[2] = 0; np_set_display(); return; }
    g_numpad_buf[len - 1] = 0;
    if (len >= 3 && g_numpad_buf[len - 2] == '.') g_numpad_buf[len - 2] = 0;
    np_set_display();
}

static void np_clear(void) {
    strcpy(g_numpad_buf, "+0");
    np_set_display();
}

static void np_toggle_sign(void) {
    g_numpad_buf[0] = (g_numpad_buf[0] == '-') ? '+' : '-';
    np_set_display();
}

static void np_btn_cb(lv_event_t *e) {
    const char *c = lv_event_get_user_data(e);
    if (strcmp(c, "OK") == 0) {
        keypad_enable(false);
        logic_set_diameter(atof(g_numpad_buf));
        if (g_modal) { lv_obj_del(g_modal); g_modal = NULL; }
        if (g_underlay) { lv_obj_del(g_underlay); g_underlay = NULL; }
        g_np_label = NULL;
        return;
    }
    if (strcmp(c, "C") == 0) { np_clear(); return; }
    if (strcmp(c, "Cancel") == 0) {
        keypad_enable(false);
        if (g_modal) { lv_obj_del(g_modal); g_modal = NULL; }
        if (g_underlay) { lv_obj_del(g_underlay); g_underlay = NULL; }
        g_np_label = NULL;
        return;
    }
    if (strcmp(c, "BS") == 0) { np_backspace(); return; }
    if (strcmp(c, "+-") == 0) { np_toggle_sign(); return; }
    if (strcmp(c, ".") == 0) {
        if (!strchr(g_numpad_buf, '.')) {
            int len = strlen(g_numpad_buf);
            if (len < 15) { g_numpad_buf[len] = '.'; g_numpad_buf[len + 1] = '0'; g_numpad_buf[len + 2] = 0; }
        }
        np_set_display();
        return;
    }
    np_place_digit(c[0]);
}

static lv_obj_t *np_make_btn(lv_obj_t *p, const char *t, const char *ev, int x, int y, int w, int h,
                              const lv_font_t *font) {
    lv_obj_t *b = lv_btn_create(p);
    lv_obj_set_pos(b, x, y); lv_obj_set_size(b, w, h);
    lv_obj_add_style(b, &s_btn, 0);
    lv_obj_t *l = lv_label_create(b); lv_label_set_text(l, t); lv_obj_center(l);
    lv_obj_add_style(l, &s_title, 0);
    if (font) lv_obj_set_style_text_font(l, font, 0);
    lv_obj_add_event_cb(b, np_btn_cb, LV_EVENT_CLICKED, (void *)(ev ? ev : t));
    return b;
}

static void show_numpad(void) {
    keypad_enable(true);
    g_underlay = underlay_create(underlay_close_cb);

    g_modal = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_modal, 460, 340); lv_obj_center(g_modal);
    lv_obj_add_style(g_modal, &s_panel, 0);
    lv_obj_set_style_pad_all(g_modal, 12, 0);
    lv_obj_set_style_border_color(g_modal, CFG_LV_HIGHLIGHT, 0);
    lv_obj_set_style_border_width(g_modal, 2, 0);
    lv_obj_set_scrollbar_mode(g_modal, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(g_modal, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *ttl = lv_label_create(g_modal);
    lv_label_set_text(ttl, "\u0414\u0418\u0410\u041C\u0415\u0422\u0420, mm");
    lv_obj_add_style(ttl, &s_title, 0);
    lv_obj_set_pos(ttl, 2, 2);

    g_np_label = lv_label_create(g_modal);
    lv_obj_add_style(g_np_label, &s_val_sp, 0);
    lv_obj_set_style_text_color(g_np_label, CFG_LV_TEXT, 0);
    lv_obj_set_pos(g_np_label, 2, 40);
    lv_obj_set_width(g_np_label, 432);
    lv_obj_set_style_text_align(g_np_label, LV_TEXT_ALIGN_RIGHT, 0);

    float v = logic_get_display_mm(AXIS_RADIAL, -1);
    snprintf(g_numpad_buf, sizeof(g_numpad_buf), "%+.3f", (double)v);
    {
        char *dot = strchr(g_numpad_buf, '.');
        if (dot) {
            int e = strlen(g_numpad_buf) - 1;
            while (e > 0 && g_numpad_buf[e] == '0') e--;
            if (g_numpad_buf[e] == '.') e--;
            g_numpad_buf[e + 1] = 0;
        }
    }
    np_set_display();

    int bx = 2, by = 107, bw = 82, bh = 46, gap = 6;
    np_make_btn(g_modal, "1", NULL, bx, by, bw, bh, NULL);
    np_make_btn(g_modal, "2", NULL, bx+bw+gap, by, bw, bh, NULL);
    np_make_btn(g_modal, "3", NULL, bx+2*(bw+gap), by, bw, bh, NULL);
    np_make_btn(g_modal, "OK", NULL, bx+3*(bw+gap), by, bw*2+gap, bh, NULL);

    np_make_btn(g_modal, "4", NULL, bx, by+bh+gap, bw, bh, NULL);
    np_make_btn(g_modal, "5", NULL, bx+bw+gap, by+bh+gap, bw, bh, NULL);
    np_make_btn(g_modal, "6", NULL, bx+2*(bw+gap), by+bh+gap, bw, bh, NULL);
    np_make_btn(g_modal, "\u041E\u0442\u043C\u0435\u043D\u0430", "Cancel",
                bx+3*(bw+gap), by+bh+gap, bw*2+gap, bh, NULL);

    np_make_btn(g_modal, "7", NULL, bx, by+2*(bh+gap), bw, bh, NULL);
    np_make_btn(g_modal, "8", NULL, bx+bw+gap, by+2*(bh+gap), bw, bh, NULL);
    np_make_btn(g_modal, "9", NULL, bx+2*(bw+gap), by+2*(bh+gap), bw, bh, NULL);
    np_make_btn(g_modal, "C", NULL, bx+3*(bw+gap), by+2*(bh+gap), bw*2+gap, bh, NULL);

    np_make_btn(g_modal, ".",  NULL, bx, by+3*(bh+gap), bw, bh, NULL);
    np_make_btn(g_modal, "0",  NULL, bx+bw+gap, by+3*(bh+gap), bw, bh, NULL);
    np_make_btn(g_modal, "+-", NULL, bx+2*(bw+gap), by+3*(bh+gap), bw, bh, NULL);
    np_make_btn(g_modal, "\uF55A", "BS", bx+3*(bw+gap), by+3*(bh+gap), bw*2+gap, bh, UI_FONT_BS);
}

static void diam_btn_cb(lv_event_t *e) { show_numpad(); }

static void phys_diam_cb(void)  { show_numpad(); }
static void phys_axial_cb(void) { logic_zero_main(AXIS_AXIAL); }

void ui_main_handle_keypad(char c)
{
    switch (c) {
    case 'A':   /* OK */
        keypad_enable(false);
        logic_set_diameter(atof(g_numpad_buf));
        if (g_modal) { lv_obj_del(g_modal); g_modal = NULL; g_np_label = NULL; }
        if (g_underlay) { lv_obj_del(g_underlay); g_underlay = NULL; }
        break;
    case 'B':   /* Cancel */
        keypad_enable(false);
        if (g_modal) { lv_obj_del(g_modal); g_modal = NULL; g_np_label = NULL; }
        if (g_underlay) { lv_obj_del(g_underlay); g_underlay = NULL; }
        break;
    case 'C':   /* Clear */
        np_clear();
        break;
    case 'D':   /* Backspace */
        np_backspace();
        break;
    case '*':   /* Decimal point */
        if (!strchr(g_numpad_buf, '.')) {
            int len = strlen(g_numpad_buf);
            if (len < 15) { g_numpad_buf[len] = '.'; g_numpad_buf[len+1] = '0'; g_numpad_buf[len+2] = 0; }
        }
        np_set_display();
        break;
    case '#':   /* Toggle sign */
        np_toggle_sign();
        break;
    default:
        if (c >= '0' && c <= '9') np_place_digit(c);
        break;
    }
}

/* ── CONFIRM DIALOG ── */
static void dlg_yes(lv_event_t *e) { logic_zero_main((axis_t)g_dlg_axis); if(g_dlg){lv_obj_del(g_dlg);g_dlg=NULL;} if(g_underlay){lv_obj_del(g_underlay);g_underlay=NULL;} }
static void dlg_no(lv_event_t *e)  { if(g_dlg){lv_obj_del(g_dlg);g_dlg=NULL;} if(g_underlay){lv_obj_del(g_underlay);g_underlay=NULL;} }
static void dlg_close_cb(lv_event_t *e) { dlg_no(e); }
static void show_confirm(axis_t a) {
    g_dlg_axis = a;
    g_underlay = underlay_create(dlg_close_cb);

    g_dlg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_dlg, 360, 150); lv_obj_center(g_dlg);
    lv_obj_add_style(g_dlg, &s_panel, 0);
    lv_obj_set_style_pad_all(g_dlg, 12, 0);
    lv_obj_set_style_border_color(g_dlg, CFG_LV_HIGHLIGHT, 0);
    lv_obj_set_style_border_width(g_dlg, 2, 0);
    lv_obj_set_scrollbar_mode(g_dlg, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(g_dlg, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *t = lv_label_create(g_dlg);
    lv_label_set_text(t, "\u041E\u0411\u041D\u0423\u041B\u0418\u0422\u042C?");
    lv_obj_add_style(t, &s_title, 0);
    lv_obj_set_style_text_color(t, CFG_LV_TEXT, 0);
    lv_obj_set_pos(t, 2, 2);

    lv_obj_t *y = lv_btn_create(g_dlg);
    lv_obj_set_size(y, 120, 46); lv_obj_align(y, LV_ALIGN_BOTTOM_LEFT, 16, -16);
    lv_obj_add_style(y, &s_btn_danger, 0);
    lv_obj_t *yl = lv_label_create(y); lv_label_set_text(yl, "\u0414\u0410"); lv_obj_center(yl);
    lv_obj_add_style(yl, &s_title, 0);
    lv_obj_add_event_cb(y, dlg_yes, LV_EVENT_CLICKED, NULL);

    lv_obj_t *n = lv_btn_create(g_dlg);
    lv_obj_set_size(n, 120, 46); lv_obj_align(n, LV_ALIGN_BOTTOM_RIGHT, -16, -16);
    lv_obj_add_style(n, &s_btn, 0);
    lv_obj_t *nl = lv_label_create(n); lv_label_set_text(nl, "\u041D\u0415\u0422"); lv_obj_center(nl);
    lv_obj_add_style(nl, &s_title, 0);
    lv_obj_add_event_cb(n, dlg_no, LV_EVENT_CLICKED, NULL);
}

/* ── BUTTON CBs ── */
static void main_zero_cb(lv_event_t *e) { show_confirm((axis_t)(uintptr_t)e->user_data); }
static void rd_cb(lv_event_t *e) { logic_toggle_rd(); }
static void sp_zero_cb(lv_event_t *e) {
    uint32_t ia = (uint32_t)(uintptr_t)e->user_data;
    logic_zero_sp((axis_t)(ia>>8), ia&0xFF);
}
static void spindle_zero_cb(lv_event_t *e) { logic_zero_spindle(); }
static void spindle_sp_cb(lv_event_t *e) {
    logic_zero_spindle_sp((int)(uintptr_t)e->user_data);
}

static lv_obj_t *make_sq_btn(lv_obj_t *p, const char *text, int x, int y,
                              lv_event_cb_t cb, void *ud, const lv_font_t *font)
{
    lv_obj_t *b = lv_btn_create(p);
    lv_obj_set_pos(b, x, y); lv_obj_set_size(b, BTN_SQ, BTN_SQ);
    lv_obj_add_style(b, &s_btn, 0);
    lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *l = lv_label_create(b); lv_label_set_text(l, text); lv_obj_center(l);
    lv_obj_add_style(l, &s_title, 0);
    if (font) lv_obj_set_style_text_font(l, font, 0);
    if (cb) lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, ud);
    return b;
}

static lv_obj_t *make_sp_plate(lv_obj_t *p, int x, int y, int w, int h)
{
    lv_obj_t *pl = lv_obj_create(p);
    lv_obj_set_pos(pl, x, y); lv_obj_set_size(pl, w, h);
    lv_obj_add_style(pl, &s_panel, 0);
    lv_obj_set_style_pad_all(pl, 6, 0);
    lv_obj_set_style_border_width(pl, 1, 0);
    lv_obj_set_scrollbar_mode(pl, LV_SCROLLBAR_MODE_OFF);
    return pl;
}

static void build_row(axis_t a, int row_y)
{
    ax_ui_t *u = &g_ax[a];
    lv_obj_t *scr = lv_scr_act();

    u->row = lv_obj_create(scr);
    lv_obj_set_pos(u->row, COL2_X, row_y);
    lv_obj_set_size(u->row, COL2_W, ROW_H);
    lv_obj_add_style(u->row, &s_panel, 0);
    lv_obj_set_style_pad_all(u->row, ROW_PAD, 0);
    lv_obj_set_style_border_width(u->row, 1, 0);
    lv_obj_set_scrollbar_mode(u->row, LV_SCROLLBAR_MODE_OFF);

    u->title = lv_label_create(u->row);
    lv_obj_set_pos(u->title, 8, TITLE_Y);
    lv_label_set_text(u->title, a == AXIS_RADIAL
        ? "\u0420\u0410\u0414\u0418\u0410\u041B\u042C\u041D\u0410\u042F (\u043C\u043C)"
        : "\u041E\u0421\u0415\u0412\u0410\u042F (\u043C\u043C)");
    lv_obj_add_style(u->title, &s_title, 0);

    u->val = lv_label_create(u->row);
    lv_obj_set_pos(u->val, 8, VALUE_Y);
    lv_obj_set_width(u->val, VAL_W);
    lv_obj_set_style_text_align(u->val, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_add_style(u->val, &s_val_big, 0);

    /* left buttons (COL1) */
    if (a == AXIS_RADIAL) {
        make_sq_btn(scr, "\u00D8", COL1_X, row_y + SP_Y1,
                    diam_btn_cb, NULL, UI_FONT_DIAM);
        lv_obj_t *db = make_sq_btn(scr, "D", COL1_X, row_y + SP_Y2,
                    rd_cb, NULL, NULL);
        u->rd_label = lv_obj_get_child(db, 0);  /* first child = label */
    } else {
        make_sq_btn(scr, "0", COL1_X, row_y + SP_Y1,
                    main_zero_cb, (void*)(uintptr_t)a, NULL);
    }

    /* setpoint plates (inside row) + right buttons (COL3) */
    for (int i = 0; i < SP_C; i++) {
        int sy = (i == 0) ? SP_Y1 : SP_Y2;
        u->sp_p[i] = make_sp_plate(u->row, SP_LOCX, sy, SP_W, SP_H);
        u->sp_v[i] = lv_label_create(u->sp_p[i]);
        lv_label_set_text(u->sp_v[i], "---.---");
        lv_obj_set_style_text_color(u->sp_v[i], CFG_LV_TEXT_DIM, 0);
        lv_obj_set_style_text_align(u->sp_v[i], LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_width(u->sp_v[i], SP_W - 14);
        lv_obj_align(u->sp_v[i], LV_ALIGN_RIGHT_MID, -2, 0);
        lv_obj_add_style(u->sp_v[i], &s_val_sp, 0);

        make_sq_btn(scr, "0", COL3_X, row_y + sy,
                    sp_zero_cb,
                    (void*)(uintptr_t)(((uint32_t)a<<8)|(uint32_t)i),
                    NULL);
    }
}

void ui_main_create(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_add_style(scr, &s_bg, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

    int y = 8;
    build_row(AXIS_RADIAL, y);
    y += ROW_H + ROW_GAP;
    build_row(AXIS_AXIAL, y);
    y += ROW_H + ROW_GAP;

    /* spindle row */
    g_sp_row = lv_obj_create(scr);
    lv_obj_set_pos(g_sp_row, COL2_X, y);
    lv_obj_set_size(g_sp_row, COL2_W, ROW_H);
    lv_obj_add_style(g_sp_row, &s_panel, 0);
    lv_obj_set_style_pad_all(g_sp_row, ROW_PAD, 0);
    lv_obj_set_style_border_width(g_sp_row, 1, 0);
    lv_obj_set_scrollbar_mode(g_sp_row, LV_SCROLLBAR_MODE_OFF);

    g_sp_title = lv_label_create(g_sp_row);
    lv_label_set_text(g_sp_title, "\u0428\u041F\u0418\u041D\u0414\u0415\u041B\u042C (\u0433\u0440\u0430\u0434.)");
    lv_obj_set_pos(g_sp_title, 8, TITLE_Y);
    lv_obj_add_style(g_sp_title, &s_title, 0);

    g_sp_val = lv_label_create(g_sp_row);
    lv_label_set_text(g_sp_val, "0.0");
    lv_obj_set_pos(g_sp_val, 8, VALUE_Y);
    lv_obj_set_width(g_sp_val, VAL_W);
    lv_obj_set_style_text_align(g_sp_val, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_add_style(g_sp_val, &s_val_big, 0);

    make_sq_btn(scr, "0", COL1_X, y + SP_Y1,
                spindle_zero_cb, NULL, NULL);

    /* angle setpoint */
    lv_obj_t *ang_p = make_sp_plate(g_sp_row, SP_LOCX, SP_Y1, SP_W, SP_H);
    g_sp_ang_v = lv_label_create(ang_p);
    lv_label_set_text(g_sp_ang_v, "0.0");
    lv_obj_set_style_text_color(g_sp_ang_v, CFG_LV_SCARLET, 0);
    lv_obj_set_style_text_font(g_sp_ang_v, UI_FONT_LARGE, 0);
    lv_obj_set_style_text_align(g_sp_ang_v, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_width(g_sp_ang_v, SP_W - 40);
    lv_obj_align(g_sp_ang_v, LV_ALIGN_RIGHT_MID, -2, 0);

    make_sq_btn(scr, "0", COL3_X, y + SP_Y1,
                spindle_sp_cb, (void*)(uintptr_t)0, NULL);

    /* spindle T2 setpoint */
    lv_obj_t *dp = make_sp_plate(g_sp_row, SP_LOCX, SP_Y2, SP_W, SP_H);
    g_sp_diam_v = lv_label_create(dp);
    lv_label_set_text(g_sp_diam_v, "---.-");
    lv_obj_set_style_text_color(g_sp_diam_v, CFG_LV_TEXT_DIM, 0);
    lv_obj_set_style_text_font(g_sp_diam_v, UI_FONT_LARGE, 0);
    lv_obj_set_style_text_align(g_sp_diam_v, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_width(g_sp_diam_v, SP_W - 40);
    lv_obj_align(g_sp_diam_v, LV_ALIGN_RIGHT_MID, -2, 0);

    make_sq_btn(scr, "0", COL3_X, y + SP_Y2,
                spindle_sp_cb, (void*)(uintptr_t)1, NULL);

    g_on_diameter_btn = phys_diam_cb;
    g_on_axial_zero_btn = phys_axial_cb;
}

/* ── UPDATE ── */
static void upd_row(ax_ui_t *u, axis_t a)
{
    state_t *st = logic_get();
    axis_data_t *ax = &st->axes[a];

    static char last_title[2][32];
    static float last_main[2] = { NAN, NAN };
    static float last_sp[2][SP_C];
    static bool  last_sp_active[2][SP_C];

    float fmain = logic_get_display_mm(a, -1);
    if (fmain != last_main[(int)a]) {
        last_main[(int)a] = fmain;
        char buf[24];
        fpos(buf, sizeof(buf), fmain);
        lv_label_set_text(u->val, buf);
    }

    for (int i = 0; i < SP_C; i++) {
        if (ax->sp[i].active) {
            float fsp = logic_get_display_mm(a, i);
            if (fsp != last_sp[(int)a][i] || !last_sp_active[(int)a][i]) {
                last_sp[(int)a][i] = fsp;
                last_sp_active[(int)a][i] = true;
                char buf[24];
                fpos(buf, sizeof(buf), fsp);
                lv_obj_set_style_text_color(u->sp_v[i], CFG_LV_SCARLET, 0);
                lv_label_set_text(u->sp_v[i], buf);
            }
        } else {
            if (last_sp_active[(int)a][i]) {
                last_sp_active[(int)a][i] = false;
                last_sp[(int)a][i] = NAN;
                lv_obj_set_style_text_color(u->sp_v[i], CFG_LV_TEXT_DIM, 0);
                lv_label_set_text(u->sp_v[i], "---.---");
            }
        }
    }
}

void ui_main_update(void)
{
    upd_row(&g_ax[0], AXIS_RADIAL);
    upd_row(&g_ax[1], AXIS_AXIAL);

    static float last_ang = NAN;
    float ang = logic_get_angle_deg();
    if (ang < 0) ang += 360.0f;
    if (ang != last_ang) {
        last_ang = ang;
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f", (double)ang);
        lv_label_set_text(g_sp_val, buf);
    }

    /* D/R button label toggle */
    {
        static bool last_rm = false;
        state_t *st = logic_get();
        bool rm = st->axes[AXIS_RADIAL].radius_mode;
        if (rm != last_rm) {
            last_rm = rm;
            if (g_ax[0].rd_label) {
                lv_label_set_text(g_ax[0].rd_label, rm ? "R" : "D");
                lv_obj_set_style_text_color(g_ax[0].rd_label,
                    rm ? CFG_LV_SCARLET : CFG_LV_TEXT, 0);
            }
        }
    }

    /* spindle T1 */
    {
        static float last_t1 = NAN;
        state_t *st = logic_get();
        if (st->spindle_sp[0].active) {
            float f = logic_get_spindle_sp_deg(0);
            if (f < 0) f += 360.0f;
            if (f != last_t1) {
                last_t1 = f;
                char buf[32];
                snprintf(buf, sizeof(buf), "%.1f", (double)f);
                lv_obj_set_style_text_color(g_sp_ang_v, CFG_LV_SCARLET, 0);
                lv_label_set_text(g_sp_ang_v, buf);
            }
        } else {
            if (last_t1 != NAN) {
                last_t1 = NAN;
                lv_obj_set_style_text_color(g_sp_ang_v, CFG_LV_TEXT_DIM, 0);
                lv_label_set_text(g_sp_ang_v, "---.-");
            }
        }
    }

    /* spindle T2 */
    {
        static float last_t2 = NAN;
        state_t *st = logic_get();
        if (st->spindle_sp[1].active) {
            float f = logic_get_spindle_sp_deg(1);
            if (f < 0) f += 360.0f;
            if (f != last_t2) {
                last_t2 = f;
                char buf[32];
                snprintf(buf, sizeof(buf), "%.1f", (double)f);
                lv_obj_set_style_text_color(g_sp_diam_v, CFG_LV_SCARLET, 0);
                lv_label_set_text(g_sp_diam_v, buf);
            }
        } else {
            if (last_t2 != NAN) {
                last_t2 = NAN;
                lv_obj_set_style_text_color(g_sp_diam_v, CFG_LV_TEXT_DIM, 0);
                lv_label_set_text(g_sp_diam_v, "---.-");
            }
        }
    }
}
