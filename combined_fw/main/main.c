#include "lcd_port.h"
#include "lvgl_port.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "main";

static void make_label(lv_obj_t *parent, const char *text, int x, int y, uint32_t bg)
{
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFF), 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_24, 0);
    lv_obj_set_style_bg_color(lbl, lv_color_hex(bg), 0);
    lv_obj_set_style_bg_opa(lbl, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(lbl, 4, 0);
    lv_obj_set_pos(lbl, x, y);
}

void app_main(void)
{
    ESP_LOGI(TAG, "LVGL v8 — 5-point calibration");

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

        // 5 labels: 4 corners + center
        make_label(scr, "TL", 0, 0, 0x8000);            // Top-Left, green bg
        make_label(scr, "TR", 700, 0, 0x001F);          // Top-Right, blue bg
        make_label(scr, "BL", 0, 430, 0xF800);          // Bottom-Left, red bg
        make_label(scr, "BR", 700, 430, 0x07E0);        // Bottom-Right, dark green bg
        make_label(scr, "C",  360, 210, 0xF81F);        // Center, magenta bg

        lvgl_port_unlock();
    }

    ESP_LOGI(TAG, "5 labels placed. Check positions.");
    while (1) { vTaskDelay(1000); }
}
