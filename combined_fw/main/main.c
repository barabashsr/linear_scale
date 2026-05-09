#include "lcd_port.h"
#include "lvgl_port.h"
#include "scale.h"
#include "spindle_enc.h"
#include "buttons.h"
#include "ui_logic.h"
#include "ui_styles.h"
#include "ui_main.h"
#include "i2c_protocol.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG_MAIN = "main";

static void i2c_scan(void)
{
    ESP_LOGI(TAG_MAIN, "I2C scan (bus %d, SDA=%d SCL=%d)...", CFG_I2C_BUS, CFG_I2C_SDA, CFG_I2C_SCL);
    for (int addr = 0x08; addr < 0x78; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(CFG_I2C_BUS, cmd, pdMS_TO_TICKS(20));
        i2c_cmd_link_delete(cmd);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG_MAIN, "  found device at 0x%02X", addr);
        }
    }
    ESP_LOGI(TAG_MAIN, "I2C scan done.");
}

static void ui_task(void *arg)
{
    int tick = 0;
    while (1) {
        int32_t axial  = scale_get_position(SCALE_AXIAL);
        int32_t radial = scale_get_position(SCALE_RADIAL);

        if (lvgl_port_lock(CFG_APP_LVGL_LOCK_MS)) {
            ui_logic_update_positions(axial, radial);
            ui_logic_get_state()->spindle_count = spindle_enc_get_count();

            if (++tick >= 4) {
                tick = 0;
                ui_logic_handle_button(buttons_read());
            }

            ui_main_update();
            lvgl_port_unlock();
        }

        vTaskDelay(pdMS_TO_TICKS(CFG_APP_POLL_MS));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG_MAIN, "Linear Scale Combined firmware starting");

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
    i2c_scan();

    ESP_ERROR_CHECK(lcd_port_init());
    ESP_ERROR_CHECK(lcd_port_bl_on());
    ESP_ERROR_CHECK(scale_init(SCALE_AXIAL, CFG_SCALE_AXIAL_A, CFG_SCALE_AXIAL_B));
    ESP_ERROR_CHECK(scale_init(SCALE_RADIAL, CFG_SCALE_RADIAL_A, CFG_SCALE_RADIAL_B));
    ESP_ERROR_CHECK(spindle_enc_init());
    ESP_ERROR_CHECK(buttons_init());

    ui_logic_init();

    if (lvgl_port_lock(CFG_APP_INIT_LOCK_MS)) {
        ui_styles_init();
        ui_main_create();
        lvgl_port_unlock();
    }

    xTaskCreate(ui_task, "ui_task", CFG_APP_TASK_STACK, NULL, CFG_APP_TASK_PRIO, NULL);
    ESP_LOGI(TAG_MAIN, "Startup complete. Main task exiting.");
    vTaskDelete(NULL);
}
