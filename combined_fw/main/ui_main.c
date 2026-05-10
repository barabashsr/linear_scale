#include "ui_main.h"
#include "ui_logic.h"
#include "ui_styles.h"
#include "config.h"
#include "i2c_protocol.h"
#include "fonts/ui_fonts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define W    800
#define H    480
#define HW   380
#define OX   CFG_UI_X_OFFSET
#define PAD  10
#define SP_C CFG_MAX_SETPOINTS

#define PLATE_W (HW - 16 - 54)
#define BTN0_W  48

typedef struct {
    lv_obj_t *title, *val, *btn_main, *btn_rd;
    lv_obj_t *sp_p[SP_C], *sp_v[SP_C], *sp_b[SP_C];
} ax_ui_t;

static ax_ui_t g_ax[2];
static lv_obj_t *g_sp_panel, *g_sp_lbl, *g_sp_ang, *g_sp_rpm, *g_sp_btn;
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
static void np_add_char(char c)
{
    int len = strlen(g_numpad_buf);
    if (len < 14) { g_numpad_buf[len] = c; g_numpad_buf[len+1] = 0; }
}

static void np_backspace(void)
{
    int len = strlen(g_numpad_buf);
    if (len > 0) g_numpad_buf[len-1] = 0;
}

static void np_toggle_sign(void)
{
    if (g_numpad_buf[0] == '-') memmove(g_numpad_buf, g_numpad_buf+1, strlen(g_numpad_buf));
    else if (g_numpad_buf[0] != 0) { memmove(g_numpad_buf+1, g_numpad_buf, strlen(g_numpad_buf)+1); g_numpad_buf[0] = '-'; }
}

static void np_update_label(void)
{
    if (g_np_label) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%s mm", g_numpad_buf[0] ? g_numpad_buf : "0");
        lv_label_set_text(g_np_label, buf);
    }
}

static void np_btn_cb(lv_event_t *e)
{
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

static void np_slider_cb(lv_event_t *e)
{
    lv_obj_t *s = lv_event_get_target(e);
    g_numpad_val = lv_slider_get_value(s) * CFG_DIAM_MAX_MM / CFG_DIAM_SLIDER_RES;
    snprintf(g_numpad_buf, sizeof(g_numpad_buf), "%.3f", (double)g_numpad_val);
    np_update_label();
    logic_set_diameter(g_numpad_val);
}

static lv_obj_t *np_make_btn(lv_obj_t *p, const char *text, int x, int y, int w, int h)
{
    lv_obj_t *b = lv_btn_create(p);
    lv_obj_set_pos(b, x, y); lv_obj_set_size(b, w, h);
    lv_obj_add_style(b, &s_btn, 0);
    lv_obj_t *l = lv_label_create(b); lv_label_set_text(l, text); lv_obj_center(l);
    lv_obj_add_style(l, &s_title, 0);
    lv_obj_add_event_cb(b, np_btn_cb, LV_EVENT_CLICKED, (void *)text);
    return b;
}

static void show_numpad(void)
{
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
static void show_confirm(axis_t a)
{
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
static void sp_zero_cb(lv_event_t *e) { uint32_t ia = (uint32_t)(uintptr_t)e->user_data; logic_zero_sp((axis_t)(ia>>8), ia&0xFF); }
static void spindle_zero_cb(lv_event_t *e) { logic_zero_spindle(); }

static lv_obj_t *make_zero_btn(lv_obj_t *p, axis_t a, int idx)
{
    lv_obj_t *b = lv_btn_create(p);
    lv_obj_set_size(b, BTN0_W, 70);
    lv_obj_add_style(b, &s_btn, 0);
    lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *l = lv_label_create(b); lv_label_set_text(l, "0"); lv_obj_center(l);
    lv_obj_add_style(l, &s_title, 0);
    lv_obj_add_event_cb(b, sp_zero_cb, LV_EVENT_CLICKED,
                        (void*)(uintptr_t)(((uint32_t)a<<8)|(uint32_t)idx));
    return b;
}

/* ── BUILD AXIS ── */
static void build_axis(lv_obj_t *p, axis_t a, int xo)
{
    ax_ui_t *u = &g_ax[a];
    int y = PAD;
    int cx = xo + PAD;

    u->title = lv_label_create(p);
    lv_obj_set_pos(u->title, cx, y);
    lv_obj_set_size(u->title, HW-16, 30);
    lv_obj_add_style(u->title, &s_title, 0);
    y += 32;

    u->val = lv_label_create(p);
    lv_obj_set_pos(u->val, cx, y);
    lv_obj_set_size(u->val, HW-16, 80);
    lv_obj_add_style(u->val, &s_val_big, 0);
    y += 88;

    /* ── main action button row ── */
    if (a == AXIS_RADIAL) {
        u->btn_main = lv_btn_create(p);
        lv_obj_set_pos(u->btn_main, cx, y);
        lv_obj_set_size(u->btn_main, 240, 52);
        lv_obj_add_style(u->btn_main, &s_btn, 0);
        lv_obj_t *l = lv_label_create(u->btn_main);
        lv_label_set_text(l, "\u0414\u0418\u0410\u041C\u0415\u0422\u0420");
        lv_obj_center(l);
        lv_obj_add_style(l, &s_title, 0);
        lv_obj_add_event_cb(u->btn_main, diam_btn_cb, LV_EVENT_CLICKED, NULL);

        u->btn_rd = lv_btn_create(p);
        lv_obj_set_pos(u->btn_rd, cx + 248, y);
        lv_obj_set_size(u->btn_rd, 112, 52);
        lv_obj_add_style(u->btn_rd, &s_btn, 0);
        lv_obj_t *rl = lv_label_create(u->btn_rd);
        lv_label_set_text(rl, "D/R");
        lv_obj_center(rl);
        lv_obj_add_style(rl, &s_title, 0);
        lv_obj_add_event_cb(u->btn_rd, rd_cb, LV_EVENT_CLICKED, NULL);
    } else {
        u->btn_main = lv_btn_create(p);
        lv_obj_set_pos(u->btn_main, cx, y);
        lv_obj_set_size(u->btn_main, HW-16, 52);
        lv_obj_add_style(u->btn_main, &s_btn, 0);
        lv_obj_t *l = lv_label_create(u->btn_main);
        lv_label_set_text(l, "\u041E\u0411\u041D\u0423\u041B\u0418\u0422\u042C");
        lv_obj_center(l);
        lv_obj_add_style(l, &s_title, 0);
        lv_obj_add_event_cb(u->btn_main, main_zero_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)a);
        u->btn_rd = NULL;
    }
    y += 62;

    /* ── setpoint rows ── */
    for (int i = 0; i < SP_C; i++) {
        u->sp_p[i] = lv_obj_create(p);
        lv_obj_add_style(u->sp_p[i], &s_panel, 0);
        lv_obj_set_size(u->sp_p[i], PLATE_W, 70);
        lv_obj_set_style_pad_all(u->sp_p[i], 6, 0);
        lv_obj_set_style_border_width(u->sp_p[i], 1, 0);
        lv_obj_set_scrollbar_mode(u->sp_p[i], LV_SCROLLBAR_MODE_OFF);

        u->sp_v[i] = lv_label_create(u->sp_p[i]);
        lv_obj_align(u->sp_v[i], LV_ALIGN_LEFT_MID, 8, 0);
        lv_obj_set_width(u->sp_v[i], PLATE_W - 16);
        lv_obj_add_style(u->sp_v[i], &s_val_sp, 0);

        u->sp_b[i] = make_zero_btn(p, a, i);

        if (a == AXIS_RADIAL) {
            lv_obj_set_pos(u->sp_b[i], cx, y);
            lv_obj_set_pos(u->sp_p[i], cx + BTN0_W + 6, y);
        } else {
            lv_obj_set_pos(u->sp_p[i], cx, y);
            lv_obj_set_pos(u->sp_b[i], cx + PLATE_W + 6, y);
        }
        y += 80;
    }
}

/* ── CREATE ── */
void ui_main_create(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_add_style(scr, &s_bg, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *div = lv_obj_create(scr);
    lv_obj_set_pos(div, OX+HW+2, 0); lv_obj_set_size(div, 2, H);
    lv_obj_set_style_bg_color(div, CFG_LV_ACCENT, 0);
    lv_obj_set_style_bg_opa(div, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(div, 0, 0);

    build_axis(scr, AXIS_RADIAL, OX);
    build_axis(scr, AXIS_AXIAL, OX + HW + 4);

    /* ── spindle panel ── */
    int sy = H - 80;

    g_sp_btn = lv_btn_create(scr);
    lv_obj_set_pos(g_sp_btn, OX+PAD, sy);
    lv_obj_set_size(g_sp_btn, BTN0_W, 70);
    lv_obj_add_style(g_sp_btn, &s_btn, 0);
    lv_obj_add_flag(g_sp_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *bl = lv_label_create(g_sp_btn); lv_label_set_text(bl, "0"); lv_obj_center(bl);
    lv_obj_add_style(bl, &s_title, 0);
    lv_obj_add_event_cb(g_sp_btn, spindle_zero_cb, LV_EVENT_CLICKED, NULL);

    int spw = W - OX*2 - PAD*2 + 8 - BTN0_W - 6;
    g_sp_panel = lv_obj_create(scr);
    lv_obj_set_pos(g_sp_panel, OX+PAD+BTN0_W+6, sy);
    lv_obj_set_size(g_sp_panel, spw, 70);
    lv_obj_add_style(g_sp_panel, &s_panel, 0);
    lv_obj_set_style_pad_all(g_sp_panel, 6, 0);
    lv_obj_set_style_border_width(g_sp_panel, 1, 0);
    lv_obj_set_scrollbar_mode(g_sp_panel, LV_SCROLLBAR_MODE_OFF);

    g_sp_lbl = lv_label_create(g_sp_panel);
    lv_label_set_text(g_sp_lbl, "\u0428\u041F\u0418\u041D\u0414\u0415\u041B\u042C");
    lv_obj_add_style(g_sp_lbl, &s_small, 0);
    lv_obj_align(g_sp_lbl, LV_ALIGN_LEFT_MID, 8, 0);

    g_sp_ang = lv_label_create(g_sp_panel);
    lv_obj_align(g_sp_ang, LV_ALIGN_CENTER, -50, 0);
    lv_obj_add_style(g_sp_ang, &s_val_act, 0);

    g_sp_rpm = lv_label_create(g_sp_panel);
    lv_obj_align(g_sp_rpm, LV_ALIGN_CENTER, 80, 0);
    lv_obj_add_style(g_sp_rpm, &s_val_act, 0);

    lv_label_set_text(g_sp_ang, "0.0\u00B0");
    lv_label_set_text(g_sp_rpm, "0 \u041E\u0411/\u041C\u0418\u041D");
}

/* ── UPDATE ── */
static void upd_axis(ax_ui_t *u, axis_t a)
{
    state_t *st = logic_get();
    axis_data_t *ax = &st->axes[a];

    lv_label_set_text(u->title, a == AXIS_RADIAL
        ? "\u0420\u0410\u0414\u0418\u0410\u041B\u042C\u041D\u0410\u042F"
        : "\u041E\u0421\u0415\u0412\u0410\u042F");

    char buf[32];
    fpos(buf, sizeof(buf), logic_get_display_mm(a, -1));
    lv_label_set_text(u->val, buf);

    for (int i = 0; i < SP_C; i++) {
        if (ax->sp[i].active) {
            fpos(buf, sizeof(buf), logic_get_display_mm(a, i));
            lv_obj_set_style_text_color(u->sp_v[i], CFG_LV_HIGHLIGHT, 0);
        } else {
            snprintf(buf, sizeof(buf), "---.---");
            lv_obj_set_style_text_color(u->sp_v[i], CFG_LV_TEXT_DIM, 0);
        }
        lv_label_set_text(u->sp_v[i], buf);
    }
}

void ui_main_update(void)
{
    upd_axis(&g_ax[0], AXIS_RADIAL);
    upd_axis(&g_ax[1], AXIS_AXIAL);

    char buf[32];
    float ang = logic_get_angle_deg();
    if (ang < 0) ang += 360.0f;
    snprintf(buf, sizeof(buf), "%.1f\u00B0", (double)ang);
    lv_label_set_text(g_sp_ang, buf);
    snprintf(buf, sizeof(buf), "%.0f \u041E\u0411/\u041C\u0418\u041D", (double)logic_get_rpm());
    lv_label_set_text(g_sp_rpm, buf);
}
