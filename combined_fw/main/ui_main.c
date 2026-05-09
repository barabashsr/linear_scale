#include "ui_main.h"
#include "ui_logic.h"
#include "ui_styles.h"
#include "i2c_protocol.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    lv_obj_t *conn_label, *title, *main_value, *main_label, *btn_zero, *btn_rd;
    lv_obj_t *sp_containers[CFG_MAX_SETPOINTS];
    lv_obj_t *sp_labels[CFG_MAX_SETPOINTS];
    lv_obj_t *sp_values[CFG_MAX_SETPOINTS];
    lv_obj_t *sp_btns[CFG_MAX_SETPOINTS];
    lv_obj_t *entry_area, *entry_ta, *entry_btn;
} axis_ui_t;

static axis_ui_t g_ui[AXIS_COUNT];
static lv_obj_t *g_dialog = NULL;
static lv_obj_t *g_spindle_val = NULL;
static lv_obj_t *g_spindle_deg = NULL;

static void format_position(char *buf, size_t len, float mm)
{
    if (mm > -CFG_APP_ZERO_THRESHOLD && mm < CFG_APP_ZERO_THRESHOLD)
        snprintf(buf, len, "+0.000");
    else if (mm < 0)
        snprintf(buf, len, "-%.3f", (double)(-mm));
    else
        snprintf(buf, len, "+%.3f", (double)mm);
}

static void confirm_main_zero_cb(lv_event_t *e)
{
    axis_t axis = (axis_t)(uintptr_t)e->user_data;
    ui_logic_zero_main(axis);
    if (g_dialog) { lv_obj_del(g_dialog); g_dialog = NULL; }
}
static void cancel_dialog_cb(lv_event_t *e)
{
    if (g_dialog) { lv_obj_del(g_dialog); g_dialog = NULL; }
}

static void show_confirm_dialog(axis_t axis)
{
    g_dialog = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_dialog, CFG_UI_DLG_W, CFG_UI_DLG_H);
    lv_obj_center(g_dialog);
    lv_obj_set_style_bg_color(g_dialog, CFG_LV_PANEL_BG, 0);
    lv_obj_set_style_border_color(g_dialog, CFG_LV_HIGHLIGHT, 0);
    lv_obj_set_style_border_width(g_dialog, CFG_UI_DLG_BORDER, 0);
    lv_obj_set_style_radius(g_dialog, CFG_UI_DLG_RADIUS, 0);

    lv_obj_t *label = lv_label_create(g_dialog);
    lv_label_set_text(label, "ОБНУЛИТЬ?");
    lv_obj_set_style_text_color(label, CFG_LV_TEXT, 0);
    lv_obj_set_style_text_font(label, CFG_FONT_SP_VALUE, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, CFG_UI_DLG_TITLE_Y);

    lv_obj_t *btn_yes = lv_btn_create(g_dialog);
    lv_obj_set_size(btn_yes, CFG_UI_DLG_BTN_W, CFG_UI_DLG_BTN_H);
    lv_obj_align(btn_yes, LV_ALIGN_BOTTOM_LEFT, CFG_UI_DLG_BTN_XOFF, CFG_UI_DLG_BTN_YOFF);
    lv_obj_add_style(btn_yes, &style_btn_danger, 0);
    lv_obj_t *by = lv_label_create(btn_yes);
    lv_label_set_text(by, "ДА"); lv_obj_center(by);
    lv_obj_add_event_cb(btn_yes, confirm_main_zero_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)axis);

    lv_obj_t *btn_no = lv_btn_create(g_dialog);
    lv_obj_set_size(btn_no, CFG_UI_DLG_BTN_W, CFG_UI_DLG_BTN_H);
    lv_obj_align(btn_no, LV_ALIGN_BOTTOM_RIGHT, -(CFG_UI_DLG_BTN_XOFF), CFG_UI_DLG_BTN_YOFF);
    lv_obj_add_style(btn_no, &style_btn, 0);
    lv_obj_t *bn = lv_label_create(btn_no);
    lv_label_set_text(bn, "НЕТ"); lv_obj_center(bn);
    lv_obj_add_event_cb(btn_no, cancel_dialog_cb, LV_EVENT_CLICKED, NULL);
}

static void btn_main_zero_cb(lv_event_t *e)
{
    show_confirm_dialog((axis_t)(uintptr_t)e->user_data);
}
static void btn_rd_toggle_cb(lv_event_t *e) { ui_logic_toggle_rd_mode(); }
static void btn_sp_zero_cb(lv_event_t *e)
{
    uint32_t ia = (uint32_t)(uintptr_t)e->user_data;
    ui_logic_zero_setpoint((axis_t)(ia >> 8), ia & 0xFF);
}
static void btn_radial_enter_cb(lv_event_t *e)
{
    float val = (float)atof(lv_textarea_get_text(g_ui[AXIS_RADIAL].entry_ta)) * 1000.0f;
    ui_logic_set_radial_main(mm_to_position(val / 1000.0f));
    lv_textarea_set_text(g_ui[AXIS_RADIAL].entry_ta, "");
}
static void btn_spindle_zero_cb(lv_event_t *e) { ui_logic_zero_spindle(); }

static axis_ui_t *build_axis(lv_obj_t *parent, axis_t axis, int x_offs)
{
    axis_ui_t *ui = &g_ui[axis];
    int y = CFG_UI_PAD;

    ui->conn_label = lv_label_create(parent);
    lv_obj_set_pos(ui->conn_label, x_offs + CFG_UI_PAD, y);
    lv_obj_set_size(ui->conn_label, CFG_UI_HALF_W - CFG_UI_MARGIN, CFG_UI_CONN_H);
    y += CFG_UI_GAP_TITLE;

    ui->title = lv_label_create(parent);
    lv_obj_set_pos(ui->title, x_offs + CFG_UI_PAD, y);
    lv_obj_set_size(ui->title, CFG_UI_HALF_W - CFG_UI_MARGIN, CFG_UI_TITLE_H);
    lv_obj_add_style(ui->title, &style_title, 0);
    y += CFG_UI_GAP_VALUE;

    ui->main_value = lv_label_create(parent);
    lv_obj_set_pos(ui->main_value, x_offs + CFG_UI_PAD, y);
    lv_obj_set_size(ui->main_value, CFG_UI_HALF_W - CFG_UI_MARGIN, CFG_UI_MAIN_VALUE_H);
    lv_obj_add_style(ui->main_value, &style_value_large, 0);
    y += CFG_UI_MAIN_VALUE_H;

    ui->main_label = lv_label_create(parent);
    lv_obj_set_pos(ui->main_label, x_offs + CFG_UI_PAD + 4, y);
    lv_obj_set_size(ui->main_label, CFG_UI_HALF_W - 24, CFG_UI_MAIN_LABEL_H);
    lv_obj_add_style(ui->main_label, &style_label_small, 0);
    y += CFG_UI_GAP_LABEL;

    ui->btn_zero = lv_btn_create(parent);
    lv_obj_set_pos(ui->btn_zero, x_offs + CFG_UI_PAD, y);
    lv_obj_set_size(ui->btn_zero, CFG_UI_HALF_W / 2 - 12, CFG_UI_BTN_ZERO_H);
    lv_obj_add_style(ui->btn_zero, &style_btn, 0);
    lv_obj_t *bz = lv_label_create(ui->btn_zero);
    lv_label_set_text(bz, "ОБНУЛИТЬ"); lv_obj_center(bz);
    lv_obj_add_event_cb(ui->btn_zero, btn_main_zero_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)axis);

    if (axis == AXIS_RADIAL) {
        ui->btn_rd = lv_btn_create(parent);
        lv_obj_set_pos(ui->btn_rd, x_offs + CFG_UI_HALF_W / 2 + 4, y);
        lv_obj_set_size(ui->btn_rd, CFG_UI_HALF_W / 2 - 12, CFG_UI_BTN_ZERO_H);
        lv_obj_add_style(ui->btn_rd, &style_btn, 0);
        lv_obj_t *rd = lv_label_create(ui->btn_rd);
        lv_label_set_text(rd, "D/R"); lv_obj_center(rd);
        lv_obj_add_event_cb(ui->btn_rd, btn_rd_toggle_cb, LV_EVENT_CLICKED, NULL);
    } else {
        ui->btn_rd = NULL;
    }
    y += CFG_UI_GAP_SP;

    for (int i = 0; i < CFG_MAX_SETPOINTS; i++) {
        ui->sp_containers[i] = lv_obj_create(parent);
        lv_obj_set_pos(ui->sp_containers[i], x_offs + CFG_UI_PAD, y);
        lv_obj_set_size(ui->sp_containers[i], CFG_UI_HALF_W - CFG_UI_MARGIN, CFG_UI_SP_ROW_H);
        lv_obj_add_style(ui->sp_containers[i], &style_panel, 0);
        lv_obj_set_style_border_width(ui->sp_containers[i], 1, 0);
        lv_obj_set_style_pad_all(ui->sp_containers[i], 2, 0);

        ui->sp_labels[i] = lv_label_create(ui->sp_containers[i]);
        lv_obj_align(ui->sp_labels[i], LV_ALIGN_LEFT_MID, 4, 0);
        lv_obj_add_style(ui->sp_labels[i], &style_label_small, 0);

        ui->sp_values[i] = lv_label_create(ui->sp_containers[i]);
        lv_obj_align(ui->sp_values[i], LV_ALIGN_LEFT_MID, 32, 0);
        lv_obj_add_style(ui->sp_values[i], &style_sp_value, 0);

        ui->sp_btns[i] = lv_btn_create(ui->sp_containers[i]);
        lv_obj_align(ui->sp_btns[i], LV_ALIGN_RIGHT_MID, -4, 0);
        lv_obj_set_size(ui->sp_btns[i], CFG_UI_SP_BTN_SIZE, CFG_UI_SP_BTN_SIZE);
        lv_obj_add_style(ui->sp_btns[i], &style_btn, 0);
        lv_obj_t *bl = lv_label_create(ui->sp_btns[i]);
        lv_label_set_text(bl, "0"); lv_obj_center(bl);
        lv_obj_add_event_cb(ui->sp_btns[i], btn_sp_zero_cb, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)(((uint32_t)axis << 8) | (uint32_t)i));
        y += CFG_UI_SP_ROW_GAP;
    }

    if (axis == AXIS_RADIAL) {
        y += 4;
        ui->entry_area = lv_obj_create(parent);
        lv_obj_set_pos(ui->entry_area, x_offs + CFG_UI_PAD, y);
        lv_obj_set_size(ui->entry_area, CFG_UI_HALF_W - CFG_UI_MARGIN, CFG_UI_ENTRY_H);
        lv_obj_add_style(ui->entry_area, &style_panel, 0);
        lv_obj_set_style_pad_all(ui->entry_area, 4, 0);

        lv_obj_t *el = lv_label_create(ui->entry_area);
        lv_label_set_text(el, "ДИАМЕТР:");
        lv_obj_set_style_text_font(el, CFG_FONT_SMALL, 0);
        lv_obj_set_style_text_color(el, CFG_LV_TEXT_DIM, 0);
        lv_obj_align(el, LV_ALIGN_LEFT_MID, 4, 0);

        ui->entry_ta = lv_textarea_create(ui->entry_area);
        lv_obj_set_size(ui->entry_ta, CFG_UI_ENTRY_TA_W, CFG_UI_ENTRY_TA_H);
        lv_obj_align(ui->entry_ta, LV_ALIGN_LEFT_MID, 68, 0);
        lv_textarea_set_one_line(ui->entry_ta, true);
        lv_textarea_set_max_length(ui->entry_ta, CFG_UI_ENTRY_TA_MAXLEN);
        lv_obj_set_style_text_font(ui->entry_ta, CFG_FONT_BTN, 0);
        lv_obj_set_style_text_color(ui->entry_ta, CFG_LV_TEXT, 0);
        lv_obj_set_style_bg_color(ui->entry_ta, CFG_LV_ACCENT, 0);

        ui->entry_btn = lv_btn_create(ui->entry_area);
        lv_obj_align(ui->entry_btn, LV_ALIGN_RIGHT_MID, -4, 0);
        lv_obj_set_size(ui->entry_btn, CFG_UI_ENTRY_OK_W, CFG_UI_ENTRY_OK_H);
        lv_obj_add_style(ui->entry_btn, &style_btn, 0);
        lv_obj_t *eb = lv_label_create(ui->entry_btn);
        lv_label_set_text(eb, "OK"); lv_obj_center(eb);
        lv_obj_add_event_cb(ui->entry_btn, btn_radial_enter_cb, LV_EVENT_CLICKED, NULL);
    }
    return ui;
}

static void update_axis_ui(axis_ui_t *ui, axis_t axis)
{
    app_state_t *st = ui_logic_get_state();
    axis_data_t *ax = &st->axes[axis];

    lv_label_set_text(ui->title, (axis == AXIS_RADIAL) ? "РАДИАЛЬНАЯ" : "ОСЕВАЯ");

    float main_mm = sp_get_mm(axis, -1);
    if (axis == AXIS_RADIAL && ax->radius_mode) main_mm *= 2.0f;

    char buf[CFG_APP_POS_BUF_LEN];
    format_position(buf, sizeof(buf), main_mm);
    lv_label_set_text(ui->main_value, buf);

    if (axis == AXIS_RADIAL) {
        float ref = position_to_mm(ax->main.ref_pos_005mm);
        if (ax->radius_mode) ref *= 2.0f;
        snprintf(buf, sizeof(buf), "главная (%.3f)", (double)ref);
    } else {
        snprintf(buf, sizeof(buf), "главная");
    }
    lv_label_set_text(ui->main_label, buf);

    for (int i = 0; i < CFG_MAX_SETPOINTS; i++) {
        if (ax->sp[i].active) {
            float v = sp_get_mm(axis, i);
            if (axis == AXIS_RADIAL && ax->radius_mode) v *= 2.0f;
            format_position(buf, sizeof(buf), v);
            lv_obj_set_style_text_color(ui->sp_values[i], CFG_LV_HIGHLIGHT, 0);
        } else {
            snprintf(buf, sizeof(buf), "---.---");
            lv_obj_set_style_text_color(ui->sp_values[i], CFG_LV_TEXT_DIM, 0);
        }
        lv_label_set_text(ui->sp_values[i], buf);
    }

    lv_obj_set_style_text_color(ui->conn_label, CFG_LV_GREEN, 0);
}

void ui_main_create(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_add_style(scr, &style_bg, 0);

    lv_obj_t *div = lv_obj_create(scr);
    lv_obj_set_pos(div, CFG_UI_HALF_W + 2, 0);
    lv_obj_set_size(div, 2, CFG_UI_FULL_H);
    lv_obj_set_style_bg_color(div, CFG_LV_ACCENT, 0);
    lv_obj_set_style_bg_opa(div, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(div, 0, 0);

    build_axis(scr, AXIS_RADIAL, 0);
    build_axis(scr, AXIS_AXIAL, CFG_UI_HALF_W + CFG_UI_AXIAL_XOFF);

    lv_obj_t *sp = lv_obj_create(scr);
    lv_obj_set_pos(sp, CFG_UI_HALF_W + CFG_UI_AXIAL_XOFF + CFG_UI_PAD,
                   CFG_UI_FULL_H - CFG_UI_SPINDLE_YOFF);
    lv_obj_set_size(sp, CFG_UI_HALF_W - CFG_UI_MARGIN, CFG_UI_SPINDLE_H);
    lv_obj_add_style(sp, &style_panel, 0);
    lv_obj_set_style_pad_all(sp, 4, 0);

    lv_obj_t *stitle = lv_label_create(sp);
    lv_label_set_text(stitle, "ШПИНДЕЛЬ");
    lv_obj_set_style_text_font(stitle, CFG_FONT_SMALL, 0);
    lv_obj_set_style_text_color(stitle, CFG_LV_TEXT_DIM, 0);
    lv_obj_align(stitle, LV_ALIGN_LEFT_MID, 4, 0);

    g_spindle_val = lv_label_create(sp);
    lv_label_set_text(g_spindle_val, "0.0");
    lv_obj_set_style_text_font(g_spindle_val, CFG_FONT_SP_VALUE, 0);
    lv_obj_set_style_text_color(g_spindle_val, CFG_LV_TEXT, 0);
    lv_obj_align(g_spindle_val, LV_ALIGN_CENTER, -6, 0);

    g_spindle_deg = lv_label_create(sp);
    lv_label_set_text(g_spindle_deg, "°");
    lv_obj_set_style_text_font(g_spindle_deg, CFG_FONT_SPINDLE, 0);
    lv_obj_set_style_text_color(g_spindle_deg, CFG_LV_TEXT, 0);
    lv_obj_align_to(g_spindle_deg, g_spindle_val, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

    lv_obj_t *sbtn = lv_btn_create(sp);
    lv_obj_align(sbtn, LV_ALIGN_RIGHT_MID, -4, 0);
    lv_obj_set_size(sbtn, CFG_UI_SPINDLE_BTN_W, CFG_UI_SPINDLE_BTN_H);
    lv_obj_add_style(sbtn, &style_btn, 0);
    lv_obj_t *sbl = lv_label_create(sbtn);
    lv_label_set_text(sbl, "0"); lv_obj_center(sbl);
    lv_obj_add_event_cb(sbtn, btn_spindle_zero_cb, LV_EVENT_CLICKED, NULL);
}

void ui_main_update(void)
{
    for (int a = 0; a < AXIS_COUNT; a++) update_axis_ui(&g_ui[a], (axis_t)a);

    float ang = sp_get_angle_deg();
    if (ang < 0) ang += 360.0f;

    char buf[CFG_APP_POS_BUF_LEN];
    snprintf(buf, sizeof(buf), "%.1f", (double)ang);
    lv_label_set_text(g_spindle_val, buf);
}
