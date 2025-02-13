#include "main.h"

void app_main (void)
{
    // esp_log_level_set("*", ESP_LOG_DEBUG);

    esp_err_t status = nvs_flash_init();

    if ((ESP_ERR_NVS_NO_FREE_PAGES == status)
        || (ESP_ERR_NVS_NEW_VERSION_FOUND == status))
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        status = nvs_flash_init();

        if (ESP_OK != status)
        {
            ESP_LOGE(MAIN_TAG,
                     "Failed to initialize nvs flash, error code: %d ",
                     status);
            goto EXIT;
        }
    }

    led_init();

    if (ESP_OK != ble_init())
    {
        ESP_LOGE(MAIN_TAG, "Failed to initialize BLE stack");
        goto EXIT;
    }

    xTaskCreate(ble_host_task, "ble_host_task", 4096, NULL, 10, NULL);

EXIT:
    return;
}