#include "ble.h"

/* Library function declarations */
void ble_store_config_init (void);

static void ble_host_config_init (void);
static void ble_on_stack_reset (int reason);
static void ble_on_stack_sync (void);
static int  ble_gap_event (struct ble_gap_event * p_event, void * p_arg);

esp_err_t ble_init (void)
{
    esp_err_t status = nimble_port_init();

    if (ESP_OK != status)
    {
        ESP_LOGE(BLE_TAG, "Failed to initialize nimble port, error code: %d", status);
        goto EXIT;
    }

    ble_host_config_init();

EXIT:
    return status;
}

// void ble_scan (void)
// {
// }

static void ble_host_config_init (void)
{
    ble_hs_cfg.reset_cb        = ble_on_stack_reset;
    ble_hs_cfg.sync_cb         = ble_on_stack_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_store_config_init();
}

static void ble_on_stack_reset (int reason)
{
    ESP_LOGI(BLE_TAG, "nimble stack reset, reset reason: %d", reason);
}

static void ble_on_stack_sync (void)
{
    ESP_LOGI(BLE_TAG, "BLE stack synchronized, starting scan...");

    struct ble_gap_disc_params scan_params = {
        .itvl          = 0x50, // Scan interval
        .window        = 0x30, // Scan window
        .filter_policy = BLE_HCI_SCAN_FILT_NO_WL,
        .limited       = 0, // General discovery mode
        .passive       = 0  // Active scanning (requests scan responses)
    };

    int status = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &scan_params, ble_gap_event, NULL);

    if (0 != status)
    {
        ESP_LOGE(BLE_TAG, "Failed to start scanning, error code: %d", status);
        vTaskDelete(NULL);
        goto EXIT;
    }

    //     // while (!(g_lens.found))
    //     // {
    //     //     // vTaskDelay(1000 / portTICK_PERIOD_MS);
    //     //     taskYIELD();
    //     //     vTaskDelay(pdMS_TO_TICKS(500));
    //     // }

    //     // ble_gap_disc_cancel();

    //     if (g_lens.found)
    //     {
    //         ESP_LOGI(BLE_TAG,
    //                  "Device found: %s, Addr: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d",
    //                  g_lens.name,
    //                  g_lens.addr[0],
    //                  g_lens.addr[1],
    //                  g_lens.addr[2],
    //                  g_lens.addr[3],
    //                  g_lens.addr[4],
    //                  g_lens.addr[5],
    //                  g_lens.rssi);
    //     }
    //     // ESP_LOGI(BLE_TAG,
    //     //          "Device found: %s, Addr: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d",
    //     //          g_lens.name,
    //     //          g_lens.addr[0],
    //     //          g_lens.addr[1],
    //     //          g_lens.addr[2],
    //     //          g_lens.addr[3],
    //     //          g_lens.addr[4],
    //     //          g_lens.addr[5],
    //     //          g_lens.rssi);

    //     // vTaskDelete(NULL);

EXIT:
    return;
}

static int ble_gap_event (struct ble_gap_event * p_event, void * p_arg)
{
    int status = -1;

    if (NULL == p_event)
    {
        ESP_LOGE(BLE_TAG, "event pointer is null!");
        goto EXIT;
    }

    struct ble_hs_adv_fields fields = { 0 };

    if (BLE_GAP_EVENT_DISC == p_event->type)
    {
        status = ble_hs_adv_parse_fields(&fields, (p_event->disc).data, (p_event->disc).length_data);

        if (0 != status)
        {
            ESP_LOGE(BLE_TAG, "failed to parse advertisement data, error code: %d", status);
            goto EXIT;
        }

        if ((0 == fields.name_len) || (NULL == fields.name))
        {
            goto EXIT;
        }

        ESP_LOGD(BLE_TAG, "Device found: %s", fields.name);

        // char device_name[32] = { 0 }; // Buffer to store device name
        // memcpy(device_name, fields.name, fields.name_len);
        // device_name[fields.name_len] = '\0'; // Null-terminate the string

        // if (strstr(device_name, "Even") && strstr(device_name, "_L_"))
        // {
        //     memcpy(p_lens->name, device_name, fields.name_len);
        //     memcpy(p_lens->addr, (p_event->disc).addr.val, 6);
        //     p_lens->rssi  = (p_event->disc).rssi;
        //     p_lens->found = true;
        //     status        = 0;
        //     ESP_LOGI(BLE_TAG,
        //              "Device found: %s, Addr: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d",
        //              g_lens.name,
        //              g_lens.addr[0],
        //              g_lens.addr[1],
        //              g_lens.addr[2],
        //              g_lens.addr[3],
        //              g_lens.addr[4],
        //              g_lens.addr[5],
        //              g_lens.rssi);
        // }
    }

EXIT:
    return status;
}