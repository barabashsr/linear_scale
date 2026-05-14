#include "keypad.h"
#include "mcp23017.h"
#include "config.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG_KP = "keypad";

#define KP_ROWS 4
#define KP_COLS 4

static const char kp_map[KP_ROWS][KP_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
};

static esp_timer_handle_t kp_timer;
static volatile bool      kp_ready;
static volatile char      kp_char;
static bool               kp_inited;

static void kp_timer_cb(void *arg)
{
    uint16_t raw = 0;
    for (int row = 0; row < KP_ROWS; row++) {
        uint8_t out = 0x0F & ~(1 << row);   /* row LOW, others HIGH */
        if (mcp23017_write(CFG_MCP23017_ADDR, MCP_OLATA, out) != ESP_OK)
            return;
        uint8_t cols;
        if (mcp23017_read(CFG_MCP23017_ADDR, MCP_GPIOA, &cols) != ESP_OK)
            return;
        cols = (cols >> 4) & 0x0F;          /* upper nibble = columns */
        for (int col = 0; col < KP_COLS; col++) {
            if (!(cols & (1 << col)))        /* column LOW = pressed */
                raw |= (1 << (row * KP_COLS + col));
        }
    }
    mcp23017_write(CFG_MCP23017_ADDR, MCP_OLATA, 0x0F);  /* restore idle */

    static uint16_t prev_raw, stable;
    static int      stable_cnt;
    if (raw == prev_raw) {
        if (++stable_cnt >= CFG_KP_DEBOUNCE) stable = raw;
    } else {
        stable_cnt = 0;
    }
    prev_raw = raw;

    static uint16_t reported;
    uint16_t new_keys = stable & ~reported;
    if (new_keys && !kp_ready) {
        for (int i = 0; i < KP_ROWS * KP_COLS; i++) {
            if (new_keys & (1 << i)) {
                kp_char = kp_map[i / KP_COLS][i % KP_COLS];
                kp_ready = true;
                break;
            }
        }
    }
    reported = stable;
}

esp_err_t keypad_init(void)
{
    if (mcp23017_init(CFG_MCP23017_ADDR) != ESP_OK) {
        mcp23017_init_gpiob(CFG_MCP23017_ADDR);
        kp_inited = false;
        return ESP_ERR_NOT_FOUND;
    }
    kp_inited = true;
    esp_timer_create_args_t ta = { .callback = kp_timer_cb, .name = "keypad" };
    esp_timer_create(&ta, &kp_timer);
    ESP_LOGI(TAG_KP, "Keypad ready");
    return ESP_OK;
}

void keypad_enable(bool enable)
{
    if (!kp_inited) return;
    if (enable) {
        kp_ready = false;
        kp_char = 0;
        esp_timer_start_periodic(kp_timer, CFG_KP_POLL_MS * 1000);
    } else {
        esp_timer_stop(kp_timer);
        mcp23017_write(CFG_MCP23017_ADDR, MCP_OLATA, 0x0F);
    }
}

bool keypad_get_char(char *c)
{
    if (!kp_ready) return false;
    *c = kp_char;
    kp_ready = false;
    return true;
}
