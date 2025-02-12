#include "main.h"

led_strip_handle_t gh_led_strip = { 0 };
hsv_t              g_hsv        = { 0, 255, 10 };

void app_main (void)
{
    esp_err_t status = nvs_flash_init();

    if ((ESP_ERR_NVS_NO_FREE_PAGES == status) || (ESP_ERR_NVS_NEW_VERSION_FOUND == status))
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        status = nvs_flash_init();

        if (ESP_OK != status)
        {
            ESP_LOGE(MAIN_TAG, "Failed to initialize nvs flash, error code: %d ", status);
            goto EXIT;
        }
    }

    if (ESP_OK != ble_init())
    {
        ESP_LOGE(MAIN_TAG, "Failed to initialize BLE stack");
        goto ERASE_NVS_EXIT;
    }

    led_configure(&gh_led_strip);

    for (;;)
    {
        led_set_hsv(gh_led_strip, &g_hsv);
        vTaskDelay(pdMS_TO_TICKS(50));
        g_hsv.hue += 5;
    }

ERASE_NVS_EXIT:
    nvs_flash_erase();
EXIT:
    return;
}