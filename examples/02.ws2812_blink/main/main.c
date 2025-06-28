#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

static const char *TAG = "ws2812_rmt";

#define BLINK_GPIO 46
#define BLINK_PERIOD 1000

static uint8_t s_led_state = 0;
static led_strip_handle_t led_strip;

static void blink_led(void)
{
    if (s_led_state)
    {
        led_strip_set_pixel(led_strip, 0, 10, 0, 0); // 红色
        led_strip_set_pixel(led_strip, 1, 0, 10, 0); // 绿色
        led_strip_set_pixel(led_strip, 2, 0, 0, 10); // 蓝色
        led_strip_refresh(led_strip);
    }
    else
    {
        led_strip_clear(led_strip);
    }
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "初始化 WS2812 LED...");
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 3,
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);
    ESP_LOGI(TAG, "WS2812 LED 初始化完成");
}



static void led_running_light(uint32_t delay_ms)
{
    for (int i = 0; i < 3; i++) {
        led_strip_clear(led_strip);
        led_strip_set_pixel(led_strip, i, 0, 10, 0);  // 绿色
        led_strip_refresh(led_strip);
        vTaskDelay(delay_ms / portTICK_PERIOD_MS);
    }
}

static void led_rainbow_cycle(uint32_t delay_ms)
{
    uint8_t r, g, b;
    for (int j = 0; j < 256; j++) {
        for (int i = 0; i < 3; i++) {
            int pos = (i * 256 / 3 + j) & 0xFF;
            if (pos < 85) {
                r = pos * 3;
                g = 255 - pos * 3;
                b = 0;
            } else if (pos < 170) {
                pos -= 85;
                r = 255 - pos * 3;
                g = 0;
                b = pos * 3;
            } else {
                pos -= 170;
                r = 0;
                g = pos * 3;
                b = 255 - pos * 3;
            }
            led_strip_set_pixel(led_strip, i, r, g, b);
        }
        led_strip_refresh(led_strip);
        vTaskDelay(delay_ms / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    configure_led();

    while (1)
    {
        ESP_LOGI(TAG, "演示：闪烁");
        s_led_state = 1;
        blink_led();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        s_led_state = 0;
        blink_led();
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        ESP_LOGI(TAG, "演示：跑马灯");
        led_running_light(300);

        ESP_LOGI(TAG, "演示：彩虹循环");
        led_rainbow_cycle(50);

    }
}