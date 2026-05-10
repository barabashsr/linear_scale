#include "lcd_port.h"
#include "lvgl_port.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "main";

enum { W = 800, H = 480, S = 40 };

static lv_obj_t *make_rect(lv_obj_t *p, int x, int y, int w, int h, uint32_t color, const char *text)
{
    lv_obj_t *r = lv_obj_create(p);
    lv_obj_set_pos(r, x, y);
    lv_obj_set_size(r, w, h);
    lv_obj_set_style_bg_color(r, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(r, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(r, 0, 0);
    lv_obj_set_style_pad_all(r, 0, 0);
    lv_obj_set_style_radius(r, 0, 0);

    lv_obj_t *l = lv_label_create(r);
    lv_label_set_text(l, text);
    lv_obj_set_style_text_color(l, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_16, 0);
    lv_obj_center(l);
    return r;
}

static lv_obj_t *make_line_h(lv_obj_t *p, int y, uint32_t color)
{
    lv_obj_t *ln = lv_obj_create(p);
    lv_obj_set_pos(ln, 0, y);
    lv_obj_set_size(ln, W, 1);
    lv_obj_set_style_bg_color(ln, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(ln, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(ln, 0, 0);
    return ln;
}

static lv_obj_t *make_line_v(lv_obj_t *p, int x, uint32_t color)
{
    lv_obj_t *ln = lv_obj_create(p);
    lv_obj_set_pos(ln, x, 0);
    lv_obj_set_size(ln, 1, H);
    lv_obj_set_style_bg_color(ln, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(ln, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(ln, 0, 0);
    return ln;
}

void app_main(void)
{
    ESP_LOGI(TAG, "DRO calibration check — offset=%d", CFG_UI_X_OFFSET);

    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 48, .scl_io_num = 47,
        .sda_pullup_en = GPIO_PULLUP_ENABLE, .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    ESP_ERROR_CHECK(i2c_param_config(0, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(0, I2C_MODE_MASTER, 0, 0, 0));

    ESP_ERROR_CHECK(lcd_port_init());
    ESP_ERROR_CHECK(lcd_port_bl_on());

    if (lvgl_port_lock(5000)) {
        lv_obj_t *scr = lv_scr_act();
        lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);

        int ox = CFG_UI_X_OFFSET;

        make_rect(scr, ox, 0, S, S, 0xFF0000, "TL");
        make_rect(scr, ox + W - S, 0, S, S, 0x00FF00, "TR");
        make_rect(scr, ox, H - S, S, S, 0x0000FF, "BL");
        make_rect(scr, ox + W - S, H - S, S, S, 0xFFFF00, "BR");
        make_rect(scr, ox + W/2 - S/2, H/2 - S/2, S, S, 0xFF00FF, "C");

        make_line_h(scr, 0, 0xFF0000);
        make_line_h(scr, H - 1, 0x00FF00);
        make_line_v(scr, ox, 0xFF0000);
        make_line_v(scr, ox + W - 1, 0x00FF00);

        lvgl_port_unlock();
    }

    ESP_LOGI(TAG, "Offset %d applied. Check markers.", CFG_UI_X_OFFSET);
    while (1) { vTaskDelay(1000); }
}
