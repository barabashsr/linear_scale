#include "ui_main.h"
#include "ui_logic.h"
#include "ui_styles.h"
#include "i2c_protocol.h"
#include <stdio.h>
#include <stdlib.h>

#define HALF_W  390
#define FULL_H  480
#define PAD     8

typedef struct {
    lv_obj_t *conn_label;
    lv_obj_t *title;
    lv_obj_t *unit_label;
    lv_obj_t *main_value;
    lv_obj_t *main_label;
    lv_obj_t *btn_zero;
    lv_obj_t *btn_rd;

    lv_obj_t *sp_containers[MAX_SETPOINTS];
    lv_obj_t *sp_labels[MAX_SETPOINTS];
    lv_obj_t *sp_values[MAX_SETPOINTS];
    lv_obj_t *sp_btns[MAX_SETPOINTS];

    lv_obj_t *entry_area;
    lv_obj_t *entry_ta;
    lv_obj_t *entry_btn;
} axis_ui_t;

static axis_ui_t g_ui[AXIS_COUNT];
static lv_obj_t *g_dialog = NULL;

static void format_position(char *buf, size_t len, float mm)
{
    if (mm > -0.0005f && mm < 0.0005f) {
        snprintf(buf, len, "+0.000");
    } else if (mm < 0) {
        snprintf(buf, len, "-%.3f", (double)(-mm));
    } else {
        snprintf(buf, len, "+%.3f", (double)mm);
    }
}

static void confirm_main_zero_cb(lv_event_t *e)
{
    axis_t axis = (axis_t)(uintptr_t)e->user_data;
    ui_logic_zero_main(axis);
    if (g_dialog) {
        lv_obj_del(g_dialog);
        g_dialog = NULL;
    }
}

static void cancel_dialog_cb(lv_event_t *e)
{
    if (g_dialog) {
        lv_obj_del(g_dialog);
        g_dialog = NULL;
    }
}

static void show_confirm_dialog(axis_t axis)
{
    g_dialog = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_dialog, 280, 120);
    lv_obj_center(g_dialog);
    lv_obj_set_style_bg_color(g_dialog, COLOR_PANEL_BG, 0);
    lv_obj_set_style_border_color(g_dialog, COLOR_HIGHLIGHT, 0);
    lv_obj_set_style_border_width(g_dialog, 2, 0);
    lv_obj_set_style_radius(g_dialog, 8, 0);

    lv_obj_t *label = lv_label_create(g_dialog);
    lv_label_set_text(label, "ОБНУЛИТЬ?");
    lv_obj_set_style_text_color(label, COLOR_TEXT, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 12);

    lv_obj_t *btn_yes = lv_btn_create(g_dialog);
    lv_obj_set_size(btn_yes, 100, 36);
    lv_obj_align(btn_yes, LV_ALIGN_BOTTOM_LEFT, 20, -12);
    lv_obj_add_style(btn_yes, &style_btn_danger, 0);
    lv_obj_t *btn_yes_label = lv_label_create(btn_yes);
    lv_label_set_text(btn_yes_label, "ДА");
    lv_obj_center(btn_yes_label);
    lv_obj_add_event_cb(btn_yes, confirm_main_zero_cb, LV_EVENT_CLICKED,
                        (void *)(uintptr_t)axis);

    lv_obj_t *btn_no = lv_btn_create(g_dialog);
    lv_obj_set_size(btn_no, 100, 36);
    lv_obj_align(btn_no, LV_ALIGN_BOTTOM_RIGHT, -20, -12);
    lv_obj_add_style(btn_no, &style_btn, 0);
    lv_obj_t *btn_no_label = lv_label_create(btn_no);
    lv_label_set_text(btn_no_label, "НЕТ");
    lv_obj_center(btn_no_label);
    lv_obj_add_event_cb(btn_no, cancel_dialog_cb, LV_EVENT_CLICKED, NULL);
}

static void btn_main_zero_cb(lv_event_t *e)
{
    axis_t axis = (axis_t)(uintptr_t)e->user_data;
    show_confirm_dialog(axis);
}

static void btn_rd_toggle_cb(lv_event_t *e)
{
    ui_logic_toggle_rd_mode();
}

static void btn_sp_zero_cb(lv_event_t *e)
{
    uint32_t idx_axis = (uint32_t)(uintptr_t)e->user_data;
    axis_t axis = (axis_t)(idx_axis >> 8);
    int idx = idx_axis & 0xFF;
    ui_logic_zero_setpoint(axis, idx);
}

static void btn_radial_enter_cb(lv_event_t *e)
{
    axis_ui_t *ui = &g_ui[AXIS_RADIAL];
    const char *text = lv_textarea_get_text(ui->entry_ta);
    float val = atof(text) * 1000.0f;
    int32_t pos = mm_to_position(val / 1000.0f);
    ui_logic_set_radial_main(pos);
    lv_textarea_set_text(ui->entry_ta, "");
}

static axis_ui_t *build_axis(lv_obj_t *parent, axis_t axis, int x_offs)
{
    axis_ui_t *ui = &g_ui[axis];
    int y = PAD;

    ui->conn_label = lv_label_create(parent);
    lv_obj_set_pos(ui->conn_label, x_offs + PAD, y);
    lv_obj_set_size(ui->conn_label, HALF_W - 16, 20);

    y += 22;
    ui->title = lv_label_create(parent);
    lv_obj_set_pos(ui->title, x_offs + PAD, y);
    lv_obj_set_size(ui->title, HALF_W - 16, 24);
    lv_obj_add_style(ui->title, &style_title, 0);

    y += 28;
    ui->main_value = lv_label_create(parent);
    lv_obj_set_pos(ui->main_value, x_offs + PAD, y);
    lv_obj_set_size(ui->main_value, HALF_W - 16, 52);
    lv_obj_add_style(ui->main_value, &style_value_large, 0);

    y += 52;
    ui->main_label = lv_label_create(parent);
    lv_obj_set_pos(ui->main_label, x_offs + PAD + 4, y);
    lv_obj_set_size(ui->main_label, HALF_W - 24, 20);
    lv_obj_add_style(ui->main_label, &style_label_small, 0);

    y += 24;
    ui->btn_zero = lv_btn_create(parent);
    lv_obj_set_pos(ui->btn_zero, x_offs + PAD, y);
    lv_obj_set_size(ui->btn_zero, HALF_W / 2 - 12, 36);
    lv_obj_add_style(ui->btn_zero, &style_btn, 0);
    lv_obj_t *bz_label = lv_label_create(ui->btn_zero);
    lv_label_set_text(bz_label, "ОБНУЛИТЬ");
    lv_obj_center(bz_label);
    lv_obj_add_event_cb(ui->btn_zero, btn_main_zero_cb, LV_EVENT_CLICKED,
                        (void *)(uintptr_t)axis);

    if (axis == AXIS_RADIAL) {
        ui->btn_rd = lv_btn_create(parent);
        lv_obj_set_pos(ui->btn_rd, x_offs + HALF_W / 2 + 4, y);
        lv_obj_set_size(ui->btn_rd, HALF_W / 2 - 12, 36);
        lv_obj_add_style(ui->btn_rd, &style_btn, 0);
        lv_obj_t *rd_label = lv_label_create(ui->btn_rd);
        lv_label_set_text(rd_label, "D/R");
        lv_obj_center(rd_label);
        lv_obj_add_event_cb(ui->btn_rd, btn_rd_toggle_cb, LV_EVENT_CLICKED, NULL);
    } else {
        ui->btn_rd = NULL;
    }

    y += 44;
    for (int i = 0; i < MAX_SETPOINTS; i++) {
        ui->sp_containers[i] = lv_obj_create(parent);
        lv_obj_set_pos(ui->sp_containers[i], x_offs + PAD, y);
        lv_obj_set_size(ui->sp_containers[i], HALF_W - 16, 44);
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
        lv_obj_set_size(ui->sp_btns[i], 32, 32);
        lv_obj_add_style(ui->sp_btns[i], &style_btn, 0);
        lv_obj_t *b_label = lv_label_create(ui->sp_btns[i]);
        lv_label_set_text(b_label, "0");
        lv_obj_center(b_label);

        uint32_t idx_axis = ((uint32_t)axis << 8) | (uint32_t)i;
        lv_obj_add_event_cb(ui->sp_btns[i], btn_sp_zero_cb, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)idx_axis);

        y += 48;
    }

    if (axis == AXIS_RADIAL) {
        y += 4;
        ui->entry_area = lv_obj_create(parent);
        lv_obj_set_pos(ui->entry_area, x_offs + PAD, y);
        lv_obj_set_size(ui->entry_area, HALF_W - 16, 40);
        lv_obj_add_style(ui->entry_area, &style_panel, 0);
        lv_obj_set_style_pad_all(ui->entry_area, 4, 0);

        lv_obj_t *el = lv_label_create(ui->entry_area);
        lv_label_set_text(el, "ДИАМЕТР:");
        lv_obj_set_style_text_font(el, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(el, COLOR_TEXT_DIM, 0);
        lv_obj_align(el, LV_ALIGN_LEFT_MID, 4, 0);

        ui->entry_ta = lv_textarea_create(ui->entry_area);
        lv_obj_set_size(ui->entry_ta, 80, 28);
        lv_obj_align(ui->entry_ta, LV_ALIGN_LEFT_MID, 68, 0);
        lv_textarea_set_one_line(ui->entry_ta, true);
        lv_textarea_set_max_length(ui->entry_ta, 8);
        lv_obj_set_style_text_font(ui->entry_ta, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(ui->entry_ta, COLOR_TEXT, 0);
        lv_obj_set_style_bg_color(ui->entry_ta, COLOR_ACCENT, 0);

        ui->entry_btn = lv_btn_create(ui->entry_area);
        lv_obj_align(ui->entry_btn, LV_ALIGN_RIGHT_MID, -4, 0);
        lv_obj_set_size(ui->entry_btn, 36, 28);
        lv_obj_add_style(ui->entry_btn, &style_btn, 0);
        lv_obj_t *eb_label = lv_label_create(ui->entry_btn);
        lv_label_set_text(eb_label, "OK");
        lv_obj_center(eb_label);
        lv_obj_add_event_cb(ui->entry_btn, btn_radial_enter_cb,
                            LV_EVENT_CLICKED, NULL);
    }

    return ui;
}

static void update_axis_ui(axis_ui_t *ui, axis_t axis)
{
    app_state_t *st = ui_logic_get_state();
    axis_data_t *ax = &st->axes[axis];

    if (axis == AXIS_RADIAL) {
        lv_label_set_text(ui->title, "РАДИАЛЬНАЯ");
    } else {
        lv_label_set_text(ui->title, "ОСЕВАЯ");
    }

    float main_mm = sp_get_mm(axis, -1);
    if (axis == AXIS_RADIAL) {
        if (ax->radius_mode) {
            main_mm *= 2.0f;
        }
    }

    char buf[32];
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

    for (int i = 0; i < MAX_SETPOINTS; i++) {
        snprintf(buf, sizeof(buf), "T%d", i + 1);
        lv_label_set_text(ui->sp_labels[i], buf);

        if (ax->sp[i].active) {
            float v = sp_get_mm(axis, i);
            if (axis == AXIS_RADIAL && ax->radius_mode) v *= 2.0f;
            format_position(buf, sizeof(buf), v);
            lv_obj_add_style(ui->sp_values[i], &style_sp_active, 0);
        } else {
            snprintf(buf, sizeof(buf), "---.---");
            lv_obj_add_style(ui->sp_values[i], &style_sp_value, 0);
        }
        lv_label_set_text(ui->sp_values[i], buf);
    }

    if (st->reader_ok) {
        lv_label_set_text(ui->conn_label, "I2C OK");
        lv_obj_add_style(ui->conn_label, &style_conn_ok, 0);
    } else {
        lv_label_set_text(ui->conn_label, "I2C НЕТ");
        lv_obj_add_style(ui->conn_label, &style_conn_err, 0);
    }
}

void ui_main_create(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_add_style(scr, &style_bg, 0);

    lv_obj_t *divider = lv_obj_create(scr);
    lv_obj_set_pos(divider, HALF_W + 2, 0);
    lv_obj_set_size(divider, 2, FULL_H);
    lv_obj_set_style_bg_color(divider, COLOR_ACCENT, 0);
    lv_obj_set_style_bg_opa(divider, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(divider, 0, 0);

    build_axis(scr, AXIS_RADIAL, 0);
    build_axis(scr, AXIS_AXIAL, HALF_W + 6);
}

void ui_main_update(void)
{
    for (int a = 0; a < AXIS_COUNT; a++) {
        update_axis_ui(&g_ui[a], (axis_t)a);
    }
}
