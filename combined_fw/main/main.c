#include "lcd_port.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"
#include <string.h>

static const char *TAG = "main";

enum { W = 800, H = 480, S = 40, OX = 16, STRIPES = 480 / 30 };

static void *g_fb0, *g_fb1, *g_panel, *g_tp;
static lv_color_t *g_lv_buf;

// ── Touch read ──
static void touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    static int last_x, last_y;
    uint16_t x, y;
    uint8_t cnt = 0;
    esp_lcd_touch_read_data(g_tp);
    if (esp_lcd_touch_get_coordinates(g_tp, &x, &y, NULL, &cnt, 1) && cnt > 0) {
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
        last_x = x; last_y = y;
    } else {
        data->point.x = last_x;
        data->point.y = last_y;
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

// ── Custom flush — all stripes to ONE buffer per frame ──
static void flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    static int render_fb = 0;
    static bool prev_was_last = true;

    if (prev_was_last) {
        render_fb = (render_fb == 0) ? 1 : 0; // new frame → switch buffer
        prev_was_last = false;
    }

    void *dst = (render_fb == 0) ? g_fb0 : g_fb1;
    uint16_t *d = (uint16_t *)dst;
    uint16_t *s = (uint16_t *)color_map;
    uint16_t pw = area->x2 - area->x1 + 1;
    for (int y = 0; y <= (area->y2 - area->y1); y++)
        memcpy(&d[(area->y1 + y) * W + area->x1], &s[y * pw], pw * 2);

    if (lv_disp_flush_is_last(drv)) {
        esp_lcd_panel_draw_bitmap(g_panel, 0, 0, W, H, dst);
        prev_was_last = true;
    }
    lv_disp_flush_ready(drv);
}

static void tick_cb(void *arg) { lv_tick_inc(2); }

static lv_obj_t *mk_rect(lv_obj_t *p, int x, int y, int w, int h, uint32_t c, const char *t)
{
    lv_obj_t *r = lv_obj_create(p); lv_obj_set_pos(r, x, y); lv_obj_set_size(r, w, h);
    lv_obj_set_style_bg_color(r, lv_color_hex(c), 0); lv_obj_set_style_bg_opa(r, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(r, 0, 0); lv_obj_set_style_pad_all(r, 0, 0); lv_obj_set_style_radius(r, 0, 0);
    lv_obj_t *l = lv_label_create(r); lv_label_set_text(l, t);
    lv_obj_set_style_text_color(l, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_16, 0); lv_obj_center(l);
    return r;
}
static void btn_cb(lv_event_t *e) { lv_label_set_text(lv_obj_get_child(lv_event_get_target(e), 0), "OK"); }
static void slider_cb(lv_event_t *e) {}

void app_main(void)
{
    ESP_LOGI(TAG, "Manual LVGL + inactive FB flush");

    i2c_config_t i2c_conf = { .mode = I2C_MODE_MASTER, .sda_io_num = 48, .scl_io_num = 47,
        .sda_pullup_en = GPIO_PULLUP_ENABLE, .scl_pullup_en = GPIO_PULLUP_ENABLE, .master.clk_speed = 400000 };
    ESP_ERROR_CHECK(i2c_param_config(0, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(0, I2C_MODE_MASTER, 0, 0, 0));

    ESP_ERROR_CHECK(lcd_port_init());
    ESP_ERROR_CHECK(lcd_port_bl_on());

    g_panel = lcd_port_get_panel_handle();
    g_fb0   = lcd_port_get_fb0();
    g_fb1   = lcd_port_get_fb1();
    g_tp    = lcd_port_get_touch_handle();

    ESP_LOGI(TAG, "Panel=%p FB0=%p FB1=%p Touch=%p", g_panel, g_fb0, g_fb1, g_tp);

    // ── LVGL init ──
    lv_init();
    g_lv_buf = heap_caps_malloc(W * 30 * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    assert(g_lv_buf);

    static lv_disp_draw_buf_t dbuf;
    lv_disp_draw_buf_init(&dbuf, g_lv_buf, NULL, W * 30);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = W; disp_drv.ver_res = H;
    disp_drv.flush_cb = flush_cb; disp_drv.draw_buf = &dbuf;
    lv_disp_drv_register(&disp_drv);

    if (g_tp) {
        static lv_indev_drv_t indev_drv;
        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        indev_drv.read_cb = touch_read;
        lv_indev_drv_register(&indev_drv);
    }

    esp_timer_handle_t tick_tmr;
    esp_timer_create(&(esp_timer_create_args_t){.callback = tick_cb, .name = "lv_tick"}, &tick_tmr);
    esp_timer_start_periodic(tick_tmr, 2000);

    // ── UI ──
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    mk_rect(scr, OX, 0, S, S, 0xFF0000, "TL");
    mk_rect(scr, W - S, 0, S, S, 0x00FF00, "TR");
    mk_rect(scr, OX, H - S, S, S, 0x0000FF, "BL");
    mk_rect(scr, W - S, H - S, S, S, 0xFFFF00, "BR");
    mk_rect(scr, W/2 - S/2, H/2 - S/2, S, S, 0xFF00FF, "C");

    lv_obj_t *slider = lv_slider_create(scr);
    lv_obj_set_size(slider, 200, 10); lv_obj_align(slider, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_slider_set_range(slider, 0, 100); lv_obj_add_event_cb(slider, slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 100, 40); lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_t *bl = lv_label_create(btn); lv_label_set_text(bl, "BTN"); lv_obj_center(bl);
    lv_obj_add_event_cb(btn, btn_cb, LV_EVENT_CLICKED, NULL);

    ESP_LOGI(TAG, "Ready.");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(lv_timer_handler()));
    }
}
