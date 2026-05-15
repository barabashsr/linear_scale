#include "lcd_port.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"
#include "scale.h"
#include "spindle_enc.h"
#include "buttons.h"
#include "keypad.h"
#include "ui_logic.h"
#include "ui_styles.h"
#include "ui_main.h"
#include "config.h"
#include <string.h>

static const char *TAG = "main";

static void *g_fb0, *g_fb1, *g_panel, *g_tp;
static lv_color_t *g_lv_buf;

static void touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    static int lx, ly;
    uint16_t x, y; uint8_t cnt = 0;
    esp_lcd_touch_read_data(g_tp);
    if (esp_lcd_touch_get_coordinates(g_tp, &x, &y, NULL, &cnt, 1) && cnt > 0) {
        data->point.x = x; data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
        lx = x; ly = y;
    } else {
        data->point.x = lx; data->point.y = ly;
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static void flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    static int render_fb = 0;
    static bool prev_was_last = true;

    if (prev_was_last) {
        render_fb = (render_fb == 0) ? 1 : 0;
        void *prev = (render_fb == 0) ? g_fb1 : g_fb0;
        void *next = (render_fb == 0) ? g_fb0 : g_fb1;
        memcpy(next, prev, CFG_LCD_H_RES * CFG_LCD_V_RES * 2);
        prev_was_last = false;
    }

    void *dst = (render_fb == 0) ? g_fb0 : g_fb1;
    uint16_t *d = (uint16_t *)dst, *s = (uint16_t *)color_map;
    uint16_t pw = area->x2 - area->x1 + 1;
    for (int y = 0; y <= (area->y2 - area->y1); y++)
        memcpy(&d[(area->y1 + y) * CFG_LCD_H_RES + area->x1], &s[y * pw], pw * 2);

    if (lv_disp_flush_is_last(drv)) {
        esp_lcd_panel_draw_bitmap(g_panel, 0, 0, CFG_LCD_H_RES, CFG_LCD_V_RES, dst);
        prev_was_last = true;
    }
    lv_disp_flush_ready(drv);
}

static void tick_cb(void *arg) { lv_tick_inc(2); }

void app_main(void)
{
    ESP_LOGI(TAG, "DRO firmware starting");

    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER, .sda_io_num = 48, .scl_io_num = 47,
        .sda_pullup_en = GPIO_PULLUP_ENABLE, .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    ESP_ERROR_CHECK(i2c_param_config(0, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(0, I2C_MODE_MASTER, 0, 0, 0));

    ESP_ERROR_CHECK(lcd_port_init());
    ESP_ERROR_CHECK(lcd_port_bl_on());

    g_panel = lcd_port_get_panel_handle();
    g_fb0   = lcd_port_get_fb0();
    g_fb1   = lcd_port_get_fb1();
    g_tp    = lcd_port_get_touch_handle();

    ESP_ERROR_CHECK(scale_init(SCALE_AXIAL, CFG_SCALE_AXIAL_A, CFG_SCALE_AXIAL_B));
    ESP_ERROR_CHECK(scale_init(SCALE_RADIAL, CFG_SCALE_RADIAL_A, CFG_SCALE_RADIAL_B));
    ESP_ERROR_CHECK(spindle_enc_init());
    ESP_ERROR_CHECK(buttons_init());
    keypad_init();
    logic_init();

    lv_init();
    g_lv_buf = heap_caps_malloc(CFG_LCD_H_RES * 240 * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    assert(g_lv_buf);

    static lv_disp_draw_buf_t dbuf;
    lv_disp_draw_buf_init(&dbuf, g_lv_buf, NULL, CFG_LCD_H_RES * 240);

    static lv_disp_drv_t dd;
    lv_disp_drv_init(&dd);
    dd.hor_res = CFG_LCD_H_RES; dd.ver_res = CFG_LCD_V_RES;
    dd.flush_cb = flush_cb; dd.draw_buf = &dbuf;
    lv_disp_drv_register(&dd);

    if (g_tp) {
        static lv_indev_drv_t id;
        lv_indev_drv_init(&id);
        id.type = LV_INDEV_TYPE_POINTER; id.read_cb = touch_read;
        lv_indev_drv_register(&id);
    }

    esp_timer_handle_t tt;
    esp_timer_create(&(esp_timer_create_args_t){.callback = tick_cb, .name = "tick"}, &tt);
    esp_timer_start_periodic(tt, 2000);

    styles_init();
    ui_main_create();

    uint32_t rpm_tick = 0;

    while (1) {
        int32_t axial = scale_get_position(SCALE_AXIAL);
        int32_t radial = scale_get_position(SCALE_RADIAL);
        logic_update(axial, radial);
        logic_handle_btn(buttons_read());

        state_t *st = logic_get();
        st->spindle_raw = spindle_enc_get_count();

        if (++rpm_tick >= 500) {
            rpm_tick = 0;
            spindle_update_rpm();
            logic_set_rpm(spindle_get_rpm());
        }

        ui_main_update();
        {
            char kp;
            if (keypad_get_char(&kp)) ui_main_handle_keypad(kp);
        }
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
