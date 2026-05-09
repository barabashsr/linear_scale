#include "aw9523.h"
#include "lvgl_port.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_touch_gt911.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG_MAIN = "main";
static esp_lcd_panel_handle_t g_panel = NULL;

IRAM_ATTR static bool on_vsync(esp_lcd_panel_handle_t panel,
                                const esp_lcd_rgb_panel_event_data_t *edata,
                                void *user_ctx)
{
    return false;
}

void app_main(void)
{
    ESP_LOGI(TAG_MAIN, "=== RAW LCD TEST — no LVGL at all ===");

    // I2C init
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CFG_I2C_SDA,
        .scl_io_num = CFG_I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = CFG_I2C_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(CFG_I2C_BUS, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(CFG_I2C_BUS, I2C_MODE_MASTER, 0, 0, 0));
    aw9523_init();

    // Backlight PWM
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = CFG_BL_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    ledc_channel_config_t ledc_ch = {
        .gpio_num = CFG_BL_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = CFG_BL_DUTY_MAX,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_ch));

    // LCD panel
    esp_lcd_rgb_panel_config_t panel_config = {
        .clk_src = LCD_CLK_SRC_PLL240M,
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
        .sram_trans_align = CFG_LCD_SRAM_ALIGN,
        .psram_trans_align = CFG_LCD_PSRAM_ALIGN,
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
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &g_panel));

    // Write test pattern BEFORE panel init — so DMA starts with clean data
    void *fb1 = NULL;
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(g_panel, 1, &fb1));
    ESP_LOGI(TAG_MAIN, "FB addr: %p", fb1);
    memset(fb1, 0, CFG_LCD_H_RES * CFG_LCD_V_RES * 2);

    uint16_t *buf = (uint16_t *)fb1;
    for (int y = 0; y < CFG_LCD_V_RES; y++) {
        for (int x = 0; x < CFG_LCD_H_RES; x++) {
            int stripe = x / 160;
            uint16_t color = 0;
            switch (stripe % 5) {
                case 0: color = 0xF800; break;
                case 1: color = 0x07E0; break;
                case 2: color = 0x001F; break;
                case 3: color = 0xFFFF; break;
                case 4: color = 0x0000; break;
            }
            buf[y * CFG_LCD_H_RES + x] = color;
        }
    }
    ESP_LOGI(TAG_MAIN, "FB filled, now init panel...");

    ESP_ERROR_CHECK(esp_lcd_panel_init(g_panel));

    esp_lcd_rgb_panel_event_callbacks_t cbs = { .on_vsync = on_vsync };
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(g_panel, &cbs, NULL));

    ESP_LOGI(TAG_MAIN, "LCD panel ready, backlight on");

    esp_lcd_panel_draw_bitmap(g_panel, 0, 0, CFG_LCD_H_RES, CFG_LCD_V_RES, fb1);
    ESP_LOGI(TAG_MAIN, "Done. No LVGL task running.");

    while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
}
