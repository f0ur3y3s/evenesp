#include "led.h"

pixel_t      g_pixel = { .is_initialized    = false,
                         .p_ble_event_group = NULL,
                         .p_led_strip       = NULL };
static hsv_t g_green = { 120, 255, 10 };
static hsv_t g_blue  = { 240, 255, 10 };
static hsv_t g_red   = { 0, 255, 10 };

void led_task (void * p_param)
{
    ESP_LOGI(LED_TAG, "LED task started");

    for (;;)
    {
        EventBits_t bits
            = xEventGroupWaitBits(g_pixel.p_ble_event_group,
                                  BLE_SCANNING | BLE_CONNECTING | BLE_CONNECTED,
                                  pdFALSE,
                                  pdFALSE,
                                  portMAX_DELAY);

        if (bits & BLE_SCANNING)
        {
            // Blink blue LED while scanning
            led_set_hsv(g_pixel.p_led_strip, &g_blue);
            vTaskDelay(pdMS_TO_TICKS(500));
            led_strip_clear(g_pixel.p_led_strip);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        else if (bits & BLE_CONNECTING)
        {
            // Blink green LED while connecting
            led_set_hsv(g_pixel.p_led_strip, &g_green);
            vTaskDelay(pdMS_TO_TICKS(200));
            led_strip_clear(g_pixel.p_led_strip);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        else if (bits & BLE_CONNECTED)
        {
            // Keep green LED on when connected
            led_set_hsv(g_pixel.p_led_strip, &g_green);
            vTaskDelay(pdMS_TO_TICKS(1000)); // Avoid busy-waiting
        }
        else
        {
            // Turn off all LEDs if no BLE activity
            led_strip_clear(g_pixel.p_led_strip);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void led_init ()
{
    if (g_pixel.is_initialized)
    {
        ESP_LOGW(LED_TAG, "LED already initialized");
        return;
    }

    led_strip_config_t led_config = {
        .strip_gpio_num   = LED_STRIP_GPIO,
        .max_leds         = LED_STRIP_NUM,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model        = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src        = RMT_CLK_SRC_DEFAULT,
        .resolution_hz  = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&led_config,
                                             &rmt_config,
                                             &(g_pixel.p_led_strip)));
    ESP_ERROR_CHECK(led_strip_clear(g_pixel.p_led_strip));

    g_pixel.p_ble_event_group = xEventGroupCreate();

    if (NULL == g_pixel.p_ble_event_group)
    {
        ESP_LOGE(LED_TAG, "Failed to create BLE event group");
    }

    xTaskCreate(led_task, "led_task", 2048, NULL, 10, NULL);

    return;
}