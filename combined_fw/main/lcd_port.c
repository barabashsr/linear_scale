#include "lcd_port.h"
#include "aw9523.h"
#include "lvgl_port.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_touch_gt911.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG_LCD = "lcd_port";
static esp_lcd_panel_handle_t g_panel = NULL;

IRAM_ATTR static bool on_vsync(esp_lcd_panel_handle_t panel,
                                const esp_lcd_rgb_panel_event_data_t *edata,
                                void *user_ctx)
{
    return lvgl_port_notify_rgb_vsync();
}

esp_err_t lcd_port_init(void)
{
    esp_err_t ret = aw9523_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG_LCD, "AW9523 not found — LCD/touch reset skipped");
    }

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
        .duty = CFG_BL_DUTY_OFF,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_ch));

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
            .flags = {
                .pclk_active_neg = CFG_LCD_PCLK_NEG,
            },
        },
        .data_width = CFG_LCD_DATA_WIDTH,
        .bits_per_pixel = CFG_LCD_BPP,
        .num_fbs = CFG_LCD_FB_COUNT,
        .bounce_buffer_size_px = CFG_LCD_H_RES * CFG_LCD_BOUNCE_H,
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
        .flags = { .fb_in_psram = CFG_LCD_FB_IN_PSRAM },
    };
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &g_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(g_panel));

    esp_lcd_touch_handle_t tp = NULL;
    esp_lcd_panel_io_handle_t tp_io = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_conf = {
        .dev_addr = 0x5B,
        .control_phase_bytes = 1,
        .dc_bit_offset = 0,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .flags = {
            .dc_low_on_data = 0,
            .disable_control_phase = 1,
        },
    };
    ret = esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)CFG_I2C_BUS,
        &tp_io_conf, &tp_io);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG_LCD, "Touch I2C IO init failed (0x%x)", ret);
        tp_io = NULL;
    }

    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = CFG_LCD_H_RES, .y_max = CFG_LCD_V_RES,
        .rst_gpio_num = GPIO_NUM_NC, .int_gpio_num = GPIO_NUM_NC,
        .levels = { .reset = 0, .interrupt = 0 },
        .flags = { .swap_xy = 0, .mirror_x = 0, .mirror_y = 0 },
    };
    if (tp_io) {
        ret = esp_lcd_touch_new_i2c_gt911(tp_io, &tp_cfg, &tp);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG_LCD, "Touch: GT911 detected at 0x5B");
        } else {
            ESP_LOGW(TAG_LCD, "Touch: GT911 not found (0x%x)", ret);
            tp = NULL;
        }
    }

    ESP_ERROR_CHECK(lvgl_port_init(g_panel, tp));

    esp_lcd_rgb_panel_event_callbacks_t cbs = {
        .on_bounce_frame_finish = on_vsync,
    };
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(g_panel, &cbs, NULL));

    ESP_LOGI(TAG_LCD, "LCD port ready (%dx%d)", CFG_LCD_H_RES, CFG_LCD_V_RES);
    return ESP_OK;
}

esp_err_t lcd_port_bl_on(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, CFG_BL_DUTY_MAX));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
    return ESP_OK;
}

esp_err_t lcd_port_bl_off(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, CFG_BL_DUTY_OFF));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
    return ESP_OK;
}
