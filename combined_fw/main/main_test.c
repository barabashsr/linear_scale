#include "lcd_port.h"
#include "lvgl_port.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG_TEST = "test";

void app_main(void)
{
    ESP_LOGI(TAG_TEST, "LCD test — solid color, no LVGL");

    ESP_ERROR_CHECK(lcd_port_init());
    ESP_ERROR_CHECK(lcd_port_bl_on());

    void *fb1 = NULL, *fb2 = NULL;
    esp_lcd_panel_handle_t panel = NULL;

    // Get panel handle — it's stored internally by lvgl_port, let's use a simpler approach
    // Actually, just use the RGB panel APIs directly
    esp_lcd_rgb_panel_config_t panel_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .timings = {
            .pclk_hz = CFG_LCD_PCLK_HZ,
            .h_res = CFG_LCD_H_RES,
            .v_res = CFG_LCD_V_RES,
            .hsync_pulse_width = CFG_LCD_HSYNC_PW,
            .hsync_back_porch = CFG_LCD_HSYNC_BP,
            .hsync_front_porch = CFG_LCD_HSYNC_FP,
            .vsync_pulse_width = CFG_LCD_VSYNC_PW,
            .vsync_back_porch = CFG_LCD_VSYNC_BP,
            .vsync_front_porch = CFG_LCD_VSYNC_FP,
        },
        .data_width = CFG_LCD_DATA_WIDTH,
        .bits_per_pixel = CFG_LCD_BPP,
        .num_fbs = 1,
        .bounce_buffer_size_px = 0,
        .dma_burst_size = 64,
        .hsync_gpio_num = CFG_LCD_HSYNC,
        .vsync_gpio_num = CFG_LCD_VSYNC,
        .de_gpio_num = CFG_LCD_DE,
        .pclk_gpio_num = CFG_LCD_PCLK,
        .disp_gpio_num = CFG_LCD_DISP,
        .data_gpio_nums = {
            CFG_LCD_D0, CFG_LCD_D1, CFG_LCD_D2, CFG_LCD_D3,
            CFG_LCD_D4, CFG_LCD_D5, CFG_LCD_D6, CFG_LCD_D7,
            CFG_LCD_D8, CFG_LCD_D9, CFG_LCD_D10, CFG_LCD_D11,
            CFG_LCD_D12, CFG_LCD_D13, CFG_LCD_D14, CFG_LCD_D15,
        },
        .flags = {
            .fb_in_psram = 1,
        },
    };
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));

    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel, 1, &fb1));

    uint16_t *buf = (uint16_t *)fb1;
    // Fill left half red, right half blue
    for (int y = 0; y < CFG_LCD_V_RES; y++) {
        for (int x = 0; x < CFG_LCD_H_RES; x++) {
            if (x < CFG_LCD_H_RES / 2) {
                buf[y * CFG_LCD_H_RES + x] = 0xF800; // Red (RGB565)
            } else {
                buf[y * CFG_LCD_H_RES + x] = 0x001F; // Blue (RGB565)
            }
        }
    }

    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, 0, 0, CFG_LCD_H_RES, CFG_LCD_V_RES, fb1));
    ESP_LOGI(TAG_TEST, "Red/blue test pattern drawn. Watch the screen.");

    while (1) { vTaskDelay(1000); }
}
