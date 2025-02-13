#include "ble.h"

/* Library function declarations */
void ble_store_config_init (void);

static void        ble_on_stack_reset (int reason);
static void        ble_on_stack_sync (void);
static int         ble_gap_event (struct ble_gap_event * p_event, void * p_arg);
inline static void format_addr (char * addr_str, uint8_t addr[]);
static void        print_conn_desc (struct ble_gap_conn_desc * desc);

static const struct ble_gap_disc_params g_scan_params = {
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
    xEventGroupClearBits(gp_pixel->p_ble_event_group, BLE_CONNECTED | BLE_SCANNING | BLE_CONNECTING);
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

static int ble_check_device (struct ble_gap_event * p_event)
{
    struct ble_hs_adv_fields fields = { 0 };
    int                      status = ble_hs_adv_parse_fields(&fields, p_event->disc.data, (p_event->disc).length_data);

    if (0 != status)
    {
        ESP_LOGE(BLE_TAG, "Failed to parse advertisement data, error code: %d", status);
        goto EXIT;
    }

    if (0 == fields.name_len)
    {
        goto EXIT;
    }

    // TODO: for test purposes, connect to NimBLE
    if (0 == strnstr((char *)(fields.name), "NimBLE_CONN", fields.name_len))
    {
        // xEventGroupClearBits(gp_pixel->p_ble_event_group, BLE_SCANNING | BLE_CONNECTED);
        // xEventGroupSetBits(gp_pixel->p_ble_event_group, BLE_CONNECTING);

        status = ble_gap_disc_cancel();

        if (status != 0 && status != BLE_HS_EALREADY)
        {
            ESP_LOGE(BLE_TAG, "Failed to stop discovery, error: %d", status);
            goto EXIT;
        }

        ESP_LOGI(BLE_TAG, "Found NimBLE device, connecting...");
        status
            = ble_gap_connect(BLE_OWN_ADDR_PUBLIC, &((p_event->disc).addr), BLE_HS_FOREVER, NULL, ble_gap_event, NULL);

        if (0 != status)
        {
            ESP_LOGE(BLE_TAG, "Failed to connect to device, error code: %d", status);
            // restart scanning

            goto EXIT;
        }
    }

EXIT:
    return status;
}

static int client_service_disc_cb (uint16_t                      conn_handle,
                                   const struct ble_gatt_error * error,
                                   const struct ble_gatt_svc *   service,
                                   void *                        arg)
{
    if (error->status != 0)
    {
        ESP_LOGE(BLE_TAG, "Service discovery error; status=%d", error->status);
        return error->status;
    }

    if (service != NULL)
    {
        ESP_LOGI(BLE_TAG,
                 "Discovered service: start_handle=%d, end_handle=%d, UUID type=%d",
                 service->start_handle,
                 service->end_handle,
                 service->uuid.u.type);
    }
    else
    {
        ESP_LOGI(BLE_TAG, "Service discovery complete");
    }
    return 0;
}

static int ble_gap_event (struct ble_gap_event * p_event, void * p_arg)
{
    int                      status = 0;
    struct ble_gap_conn_desc desc   = { 0 };

    switch (p_event->type)
    {
        case BLE_GAP_EVENT_DISC: {
            if (0 != ble_check_device(p_event))
            {
                ESP_LOGE(BLE_TAG, "Failed to check device");
            }
            // ESP_LOGI(BLE_TAG, "Device found");
        }
        break;
        case BLE_GAP_EVENT_CONNECT: {
            if (0 == (p_event->connect).status)
            {
                status = ble_gap_conn_find((p_event->connect).conn_handle, &desc);

                if (0 != status)
                {
                    ESP_LOGE(BLE_TAG, "Failed to find connection descriptor, error code: %d", status);
                    break;
                }

                print_conn_desc(&desc);

                struct ble_gap_upd_params params = { .itvl_min            = desc.conn_itvl,
                                                     .itvl_max            = desc.conn_itvl,
                                                     .latency             = 3,
                                                     .supervision_timeout = desc.supervision_timeout };

                status = ble_gap_update_params((p_event->connect).conn_handle, &params);

                status = ble_gattc_disc_all_svcs((p_event->connect).conn_handle, client_service_disc_cb, NULL);

                if (0 == status)
                {
                    xEventGroupClearBits(gp_pixel->p_ble_event_group, BLE_CONNECTED | BLE_SCANNING | BLE_CONNECTING);
                    xEventGroupSetBits(gp_pixel->p_ble_event_group, BLE_CONNECTED);
                }
                else
                {
                    ESP_LOGE(BLE_TAG, "Failed to update connection parameters, error code: %d", status);

                    status = ble_gap_terminate((p_event->connect).conn_handle, BLE_ERR_REM_USER_CONN_TERM);
                }
            }
            else
            {
                ESP_LOGE("BLE", "Connection failed, restarting scan");
                xEventGroupClearBits(gp_pixel->p_ble_event_group, BLE_CONNECTED | BLE_SCANNING | BLE_CONNECTING);
                xEventGroupSetBits(gp_pixel->p_ble_event_group, BLE_SCANNING);

                status = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &g_scan_params, ble_gap_event, NULL);

                if (0 != status)
                {
                    ESP_LOGE("BLE", "Failed to restart scan, error code: %d", status);
                }
            }
        }
        break;

        case BLE_GAP_EVENT_DISC_COMPLETE: {
            ESP_LOGI("BLE", "Scan complete, connecting...");
            xEventGroupClearBits(gp_pixel->p_ble_event_group, BLE_CONNECTED | BLE_SCANNING | BLE_CONNECTING);
            xEventGroupSetBits(gp_pixel->p_ble_event_group, BLE_CONNECTING);
            // attempt to connect to a chosen device
        }
        break;

        case BLE_GAP_EVENT_DISCONNECT: {
            ESP_LOGI("BLE", "Disconnected, restarting scan");
            xEventGroupClearBits(gp_pixel->p_ble_event_group, BLE_CONNECTED | BLE_SCANNING | BLE_CONNECTING);
            xEventGroupSetBits(gp_pixel->p_ble_event_group, BLE_SCANNING);

            status = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &g_scan_params, ble_gap_event, NULL);

            if (0 != status)
            {
                ESP_LOGE("BLE", "Failed to restart scan, error code: %d", status);
            }
        }
        break;

        default:
            break;
    }

    return status;
}

inline static void format_addr (char * addr_str, uint8_t addr[])
{
    sprintf(addr_str, "%02X:%02X:%02X:%02X:%02X:%02X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

static void print_conn_desc (struct ble_gap_conn_desc * desc)
{
    char addr_str[18] = { 0 };

    /* Connection handle */
    ESP_LOGI(BLE_TAG, "connection handle: %d", desc->conn_handle);

    /* Local ID address */
    format_addr(addr_str, desc->our_id_addr.val);
    ESP_LOGI(BLE_TAG, "device id address: type=%d, value=%s", desc->our_id_addr.type, addr_str);

    /* Peer ID address */
    format_addr(addr_str, desc->peer_id_addr.val);
    ESP_LOGI(BLE_TAG, "peer id address: type=%d, value=%s", desc->peer_id_addr.type, addr_str);

    /* Connection info */
    ESP_LOGI(BLE_TAG,
             "conn_itvl=%d, conn_latency=%d, supervision_timeout=%d, "
             "encrypted=%d, authenticated=%d, bonded=%d\n",
             desc->conn_itvl,
             desc->conn_latency,
             desc->supervision_timeout,
             desc->sec_state.encrypted,
             desc->sec_state.authenticated,
             desc->sec_state.bonded);
}
