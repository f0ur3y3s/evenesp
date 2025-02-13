#include "led.h"

/**
 * @brief Global pixel_t struct to hold the LED strip and event group.
 *
 */
pixel_t g_pixel
    = { .is_initialized    = false,
        .p_ble_event_group = NULL,
        .p_led_strip       = NULL,
        .status = { .wifi.status = { 0, 0, 0 }, .ble.status = { 0, 0, 0 } } };

static hsv_t g_green  = { 120, 255, 25 };
static hsv_t g_blue   = { 210, 255, 25 };
static hsv_t g_red    = { 0, 255, 25 };
static hsv_t g_orange = { 30, 255, 25 };
static hsv_t g_purple = { 280, 255, 25 };
static hsv_t g_yellow = { 60, 255, 25 };

/**
 * @brief Set the status of the LED strip based on the event bits.
 *
 * @param p_bits Pointer to event bits
 */
static void led_set_status (EventBits_t * p_bits);

/*
 * BLE should be orange and purple for scanning and connecting, respectively.
 * WIFI should be blue and yellow for scanning and connecting, respectively.
 *
 */

void led_task (void * p_param)
{
    ESP_LOGI(LED_TAG, "LED task started");

    for (;;)
    {
        EventBits_t bits = xEventGroupWaitBits(g_pixel.p_ble_event_group,
                                               LED_ALL,
                                               pdFALSE,
                                               pdFALSE,
                                               portMAX_DELAY);

        led_set_status(&bits);

        led_set_hsv(g_pixel.p_led_strip, &g_pixel.status.ble.status);
        vTaskDelay(pdMS_TO_TICKS(500));
        led_set_hsv(g_pixel.p_led_strip, &g_pixel.status.wifi.status);
        vTaskDelay(pdMS_TO_TICKS(500));
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

    g_pixel.status.wifi.status = g_red;
    g_pixel.status.ble.status  = g_red;

    xTaskCreate(led_task, "led_task", 2048, NULL, 10, NULL);

    return;
}

static void led_set_status (EventBits_t * p_bits)
{
    if (*p_bits & BLE_SCANNING)
    {
        g_pixel.status.ble.status = g_purple;
    }
    else if (*p_bits & BLE_CONNECTING)
    {
        g_pixel.status.ble.status = g_orange;
    }
    else if (*p_bits & BLE_CONNECTED)
    {
        g_pixel.status.ble.status = g_green;
    }
    else
    {
        g_pixel.status.ble.status = g_red;
    }

    if (*p_bits & WIFI_SCANNING)
    {
        g_pixel.status.wifi.status = g_blue;
    }
    else if (*p_bits & WIFI_CONNECTING)
    {
        g_pixel.status.wifi.status = g_yellow;
    }
    else if (*p_bits & WIFI_CONNECTED)
    {
        g_pixel.status.wifi.status = g_green;
    }
    else
    {
        g_pixel.status.wifi.status = g_red;
    }
}
