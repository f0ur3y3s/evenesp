#include "main.h"

pixel_t g_pixel = { 0 };
hsv_t   g_hsv   = { 0, 255, 10 };

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

    led_init(&g_pixel);

    if (ESP_OK != ble_init(&g_pixel))
    {
        ESP_LOGE(MAIN_TAG, "Failed to initialize BLE stack");
        // goto ERASE_NVS_EXIT;
        goto EXIT;
    }

    ble_host_config_init();

    xTaskCreate(ble_host_task, "ble_host_task", 4096, NULL, 10, NULL);
    // for (;;)
    // {
    //     // led_set_hsv(g_pixel.p_led_strip, &g_hsv);
    //     // vTaskDelay(pdMS_TO_TICKS(50));
    //     // g_hsv.hue += 5;
    // }

// ERASE_NVS_EXIT:
//     nvs_flash_erase();
EXIT:
    return;
}