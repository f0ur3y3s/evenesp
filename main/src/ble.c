#include "ble.h"

/* Library function declarations */
void ble_store_config_init (void);

// static void ble_host_config_init (void);
static void ble_on_stack_reset (int reason);
static void ble_on_stack_sync (void);
static int  ble_gap_event (struct ble_gap_event * p_event, void * p_arg);

static struct ble_gap_disc_params g_scan_params = {
    .itvl          = 0x50, // Scan interval
    .window        = 0x30, // Scan window
    .filter_policy = BLE_HCI_SCAN_FILT_NO_WL,
    .limited       = 0, // General discovery mode
    .passive       = 0  // Active scanning (requests scan responses)
};

static pixel_t * gp_pixel = NULL;

esp_err_t ble_init (pixel_t * p_pixel)
{
    esp_err_t status = nimble_port_init();

    if (ESP_OK != status)
    {
        ESP_LOGE(BLE_TAG, "Failed to initialize nimble port, error code: %d", status);
        goto EXIT;
    }

    gp_pixel = p_pixel;

EXIT:
    return status;
}

void ble_host_task (void * p_param)
{
    ESP_LOGI(BLE_TAG, "Starting BLE host task");
    nimble_port_run();
    vTaskDelete(NULL);
}

void ble_host_config_init (void)
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

    xEventGroupSetBits(gp_pixel->p_ble_event_group, BLE_SCANNING);
    int status = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &g_scan_params, ble_gap_event, NULL);

    if (0 != status)
    {
        ESP_LOGE(BLE_TAG, "Failed to start scanning, error code: %d", status);
        vTaskDelete(NULL);
        goto EXIT;
    }

EXIT:
    return;
}

static int ble_gap_event (struct ble_gap_event * p_event, void * p_arg)
{
    switch (p_event->type)
    {
        case BLE_GAP_EVENT_DISC:
            // ESP_LOGI(BLE_TAG, "Device found");
            break;
        case BLE_GAP_EVENT_CONNECT:
            if (0 == p_event->connect.status)
            {
                ESP_LOGI("BLE", "Connected to device");
                xEventGroupClearBits(gp_pixel->p_ble_event_group, BLE_SCANNING | BLE_CONNECTING);
                xEventGroupSetBits(gp_pixel->p_ble_event_group, BLE_CONNECTED);
            }
            else
            {
                ESP_LOGE("BLE", "Connection failed, restarting scan");
                xEventGroupClearBits(gp_pixel->p_ble_event_group, BLE_CONNECTING);
                xEventGroupSetBits(gp_pixel->p_ble_event_group, BLE_SCANNING);
                ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &g_scan_params, ble_gap_event, NULL);
            }
            break;

        case BLE_GAP_EVENT_DISC_COMPLETE:
            ESP_LOGI("BLE", "Scan complete, connecting...");
            xEventGroupClearBits(gp_pixel->p_ble_event_group, BLE_SCANNING);
            xEventGroupSetBits(gp_pixel->p_ble_event_group, BLE_CONNECTING);
            // attempt to connect to a chosen device
            break;

        default:
            break;
    }

    return 0;
}