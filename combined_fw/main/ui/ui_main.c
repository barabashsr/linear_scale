#include "ui_main.h"
#include "ui_logic.h"
#include "ui_styles.h"
#include "config.h"
#include "i2c_protocol.h"
#include "ui_fonts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    lv_obj_t *sp_p[SP_C], *sp_v[SP_C];
} ax_ui_t;

static ax_ui_t g_ax[2];

static lv_obj_t *g_sp_row, *g_sp_title, *g_sp_val;
static lv_obj_t *g_sp_ang_v, *g_sp_rpm_v, *g_sp_rpm_u;

static lv_obj_t *g_dlg, *g_modal;
static int g_dlg_axis;
static float g_numpad_val;
static char g_numpad_buf[16];
static lv_obj_t *g_np_label;

static void fpos(char *b, int sz, float mm)
{
    if (mm > -0.0005f && mm < 0.0005f) snprintf(b, sz, "+0.000");
    else if (mm < 0) snprintf(b, sz, "-%.3f", (double)(-mm));
    else snprintf(b, sz, "+%.3f", (double)mm);
}

/* ── NUMPAD MODAL ── */
static void np_add_char(char c) {
    int len = strlen(g_numpad_buf);
    if (len < 14) { g_numpad_buf[len] = c; g_numpad_buf[len+1] = 0; }
}
static void np_backspace(void) {
    int len = strlen(g_numpad_buf);
    if (len > 0) g_numpad_buf[len-1] = 0;
}
static void np_toggle_sign(void) {
    if (g_numpad_buf[0] == '-') memmove(g_numpad_buf, g_numpad_buf+1, strlen(g_numpad_buf));
    else if (g_numpad_buf[0] != 0) { memmove(g_numpad_buf+1, g_numpad_buf, strlen(g_numpad_buf)+1); g_numpad_buf[0] = '-'; }
}
static void np_update_label(void) {
    if (g_np_label) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%s mm", g_numpad_buf[0] ? g_numpad_buf : "0");
        lv_label_set_text(g_np_label, buf);
    }
}
static void np_btn_cb(lv_event_t *e) {
    const char *c = lv_event_get_user_data(e);
    if (strcmp(c, "OK") == 0) {
        float v = atof(g_numpad_buf);
        logic_set_diameter(v);
        if (g_modal) { lv_obj_del(g_modal); g_modal = NULL; g_np_label = NULL; }
        return;
    }
    if (strcmp(c, "BS") == 0) { np_backspace(); }
    else if (strcmp(c, "+-") == 0) { np_toggle_sign(); }
    else { np_add_char(c[0]); }
    np_update_label();
    logic_set_diameter(atof(g_numpad_buf));
}
static void np_slider_cb(lv_event_t *e) {
    lv_obj_t *s = lv_event_get_target(e);
    g_numpad_val = lv_slider_get_value(s) * CFG_DIAM_MAX_MM / CFG_DIAM_SLIDER_RES;
    snprintf(g_numpad_buf, sizeof(g_numpad_buf), "%.3f", (double)g_numpad_val);
    np_update_label();
    logic_set_diameter(g_numpad_val);
}
static lv_obj_t *np_make_btn(lv_obj_t *p, const char *t, int x, int y, int w, int h) {
    lv_obj_t *b = lv_btn_create(p);
    lv_obj_set_pos(b, x, y); lv_obj_set_size(b, w, h);
    lv_obj_add_style(b, &s_btn, 0);
    lv_obj_t *l = lv_label_create(b); lv_label_set_text(l, t); lv_obj_center(l);
    lv_obj_add_style(l, &s_title, 0);
    lv_obj_add_event_cb(b, np_btn_cb, LV_EVENT_CLICKED, (void *)t);
    return b;
}
static void show_numpad(void) {
    g_modal = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_modal, 360, 340); lv_obj_center(g_modal);
    lv_obj_add_style(g_modal, &s_panel, 0);
    lv_obj_set_style_border_color(g_modal, CFG_LV_HIGHLIGHT, 0);
    lv_obj_set_style_border_width(g_modal, 2, 0);

    lv_obj_t *t = lv_label_create(g_modal);
    lv_label_set_text(t, "\u0414\u0418\u0410\u041C\u0415\u0422\u0420, mm");
    lv_obj_add_style(t, &s_title, 0); lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 8);

    g_numpad_val = logic_get()->diam_value;
    snprintf(g_numpad_buf, sizeof(g_numpad_buf), "%.3f", (double)g_numpad_val);
    g_np_label = lv_label_create(g_modal);
    lv_label_set_text(g_np_label, "-");
    lv_obj_add_style(g_np_label, &s_val_big, 0);
    lv_obj_align(g_np_label, LV_ALIGN_TOP_MID, 0, 36);
    np_update_label();

    lv_obj_t *sl = lv_slider_create(g_modal);
    lv_obj_set_size(sl, 320, 10); lv_obj_align(sl, LV_ALIGN_TOP_MID, 0, 80);
    lv_slider_set_range(sl, 0, CFG_DIAM_SLIDER_RES);
    lv_slider_set_value(sl, (int32_t)(g_numpad_val / CFG_DIAM_MAX_MM * CFG_DIAM_SLIDER_RES), LV_ANIM_OFF);
    lv_obj_add_event_cb(sl, np_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

    int bx = 14, by = 100, bw = 78, bh = 44, gap = 4;
    const char *keys[] = {"7","8","9","BS", "4","5","6","+-", "1","2","3",".", "0","OK",NULL};
    for (int i = 0; keys[i]; i++) {
        int col = i % 4, row = i / 4;
        int kw = (strcmp(keys[i],"OK")==0) ? bw*2+gap : bw;
        np_make_btn(g_modal, keys[i], bx+col*(bw+gap), by+row*(bh+gap), kw, bh);
    }
}
static void diam_btn_cb(lv_event_t *e) { show_numpad(); }

/* ── CONFIRM DIALOG ── */
static void dlg_yes(lv_event_t *e) { logic_zero_main((axis_t)g_dlg_axis); if(g_dlg){lv_obj_del(g_dlg);g_dlg=NULL;} }
static void dlg_no(lv_event_t *e)  { if(g_dlg){lv_obj_del(g_dlg);g_dlg=NULL;} }
static void show_confirm(axis_t a) {
    g_dlg_axis = a;
    g_dlg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_dlg, 260, 110); lv_obj_center(g_dlg);
    lv_obj_add_style(g_dlg, &s_panel, 0);
    lv_obj_set_style_border_color(g_dlg, CFG_LV_HIGHLIGHT, 0);
    lv_obj_set_style_border_width(g_dlg, 2, 0);

    lv_obj_t *t = lv_label_create(g_dlg);
    lv_label_set_text(t, "\u041E\u0411\u041D\u0423\u041B\u0418\u0422\u042C?");
    lv_obj_add_style(t, &s_val_sp, 0); lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t *y = lv_btn_create(g_dlg);
    lv_obj_set_size(y, 90, 38); lv_obj_align(y, LV_ALIGN_BOTTOM_LEFT, 20, -12);
    lv_obj_add_style(y, &s_btn_danger, 0);
    lv_obj_t *yl = lv_label_create(y); lv_label_set_text(yl, "\u0414\u0410"); lv_obj_center(yl);
    lv_obj_add_style(yl, &s_title, 0);
    lv_obj_add_event_cb(y, dlg_yes, LV_EVENT_CLICKED, NULL);

    lv_obj_t *n = lv_btn_create(g_dlg);
    lv_obj_set_size(n, 90, 38); lv_obj_align(n, LV_ALIGN_BOTTOM_RIGHT, -20, -12);
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
        make_sq_btn(scr, "D/R", COL1_X, row_y + SP_Y2,
                    rd_cb, NULL, NULL);
    } else {
        make_sq_btn(scr, "0", COL1_X, row_y + SP_Y1,
                    main_zero_cb, (void*)(uintptr_t)a, NULL);
    }

    /* setpoint plates (inside row) + right buttons (COL3) */
    for (int i = 0; i < SP_C; i++) {
        int sy = (i == 0) ? SP_Y1 : SP_Y2;
        u->sp_p[i] = make_sp_plate(u->row, SP_LOCX, sy, SP_W, SP_H);
        u->sp_v[i] = lv_label_create(u->sp_p[i]);
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
                spindle_zero_cb, NULL, NULL);

    /* RPM */
    lv_obj_t *rpm_p = make_sp_plate(g_sp_row, SP_LOCX, SP_Y2, SP_W, SP_H);
    g_sp_rpm_v = lv_label_create(rpm_p);
    lv_label_set_text(g_sp_rpm_v, "0");
    lv_obj_set_style_text_color(g_sp_rpm_v, CFG_LV_SCARLET, 0);
    lv_obj_set_style_text_font(g_sp_rpm_v, UI_FONT_LARGE, 0);
    lv_obj_align(g_sp_rpm_v, LV_ALIGN_RIGHT_MID, -8, 0);

    g_sp_rpm_u = lv_label_create(rpm_p);
    lv_label_set_text(g_sp_rpm_u, "RPM");
    lv_obj_set_style_text_color(g_sp_rpm_u, CFG_LV_SCARLET, 0);
    lv_obj_set_style_text_font(g_sp_rpm_u, UI_FONT_TITLE, 0);
    lv_obj_align_to(g_sp_rpm_u, g_sp_rpm_v, LV_ALIGN_OUT_LEFT_MID, -4, 0);
}

/* ── UPDATE ── */
static void upd_row(ax_ui_t *u, axis_t a)
{
    state_t *st = logic_get();
    axis_data_t *ax = &st->axes[a];

    lv_label_set_text(u->title, a == AXIS_RADIAL
        ? "\u0420\u0410\u0414\u0418\u0410\u041B\u042C\u041D\u0410\u042F (\u043C\u043C)"
        : "\u041E\u0421\u0415\u0412\u0410\u042F (\u043C\u043C)");

    char buf[24];
    fpos(buf, sizeof(buf), logic_get_display_mm(a, -1));
    lv_label_set_text(u->val, buf);

    for (int i = 0; i < SP_C; i++) {
        if (ax->sp[i].active) {
            fpos(buf, sizeof(buf), logic_get_display_mm(a, i));
            lv_obj_set_style_text_color(u->sp_v[i], CFG_LV_SCARLET, 0);
        } else {
            snprintf(buf, sizeof(buf), "---.---");
            lv_obj_set_style_text_color(u->sp_v[i], CFG_LV_TEXT_DIM, 0);
        }
        lv_label_set_text(u->sp_v[i], buf);
    }
}

void ui_main_update(void)
{
    upd_row(&g_ax[0], AXIS_RADIAL);
    upd_row(&g_ax[1], AXIS_AXIAL);

    char buf[32];
    float ang = logic_get_angle_deg();
    if (ang < 0) ang += 360.0f;
    snprintf(buf, sizeof(buf), "%.1f", (double)ang);
    lv_label_set_text(g_sp_val, buf);
    lv_label_set_text(g_sp_ang_v, buf);

    snprintf(buf, sizeof(buf), "%.0f", (double)logic_get_rpm());
    lv_label_set_text(g_sp_rpm_v, buf);
}
