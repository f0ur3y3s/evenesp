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

    // led should track current state of device
    led_init();

    // create wifi task
    wifi_init();

    // create glasses (ble) task
    if (ESP_OK != ble_init())
    {
        ESP_LOGE(MAIN_TAG, "Failed to initialize BLE stack");
        goto EXIT;
    }

    // glasses (ble) wait for data from device
    // parse data and send to wifi task
    // wait for response from wifi task

    // wifi send data to api
    // wait for response from api
    // send response to glasses (ble) task

    // send response to device

EXIT:
    return;
}