#include "ble.h"

/* Library function declarations */
void ble_store_config_init (void);

static void        ble_on_stack_reset (int reason);
static void        ble_on_stack_sync (void);
static int         ble_gap_event (struct ble_gap_event * p_event, void * p_arg);
static void        ble_host_config_init (void);
static void        print_conn_desc (struct ble_gap_conn_desc * desc);
inline static void format_addr (char * addr_str, uint8_t addr[]);

static const struct ble_gap_disc_params g_scan_params = {
    .itvl          = 0x50,
    .window        = 0x30,
    .filter_policy = BLE_HCI_SCAN_FILT_NO_WL,
    .limited       = 0,
    .passive       = 0 //
};

static bool    g_is_connecting = false;
static uint8_t g_own_addr_type = 0;

esp_err_t ble_init (void)
{
    esp_err_t status = nimble_port_init();

    if (ESP_OK == status)
    {
        ble_host_config_init();
    }
    else
    {
        ESP_LOGE(BLE_TAG,
                 "Failed to initialize nimble port, error code: %d",
                 status);
    }

    return status;
}

void ble_host_task (void * p_param)
{
    ESP_LOGI(BLE_TAG, "Starting BLE host task");
    nimble_port_run();
    vTaskDelete(NULL);
}

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
    xEventGroupClearBits(g_pixel.p_ble_event_group, BLE_ALL);
    xEventGroupSetBits(g_pixel.p_ble_event_group, BLE_SCANNING);
    ble_hs_id_infer_auto(0, &g_own_addr_type);

    int status = ble_gap_disc(g_own_addr_type,
                              BLE_HS_FOREVER,
                              &g_scan_params,
                              ble_gap_event,
                              NULL);

    if (0 != status)
    {
        ESP_LOGE(BLE_TAG, "Failed to start scanning, error code: %d", status);
        vTaskDelete(NULL);
    }

    return;
}

static int ble_gap_event (struct ble_gap_event * p_event, void * p_arg)
{
    switch (p_event->type)
    {
        case BLE_GAP_EVENT_DISC: {
            if (g_is_connecting) // If already connecting, ignore
            {
                return 0;
            }

            // TODO: figure out a better way to parse advertisement data
            struct ble_hs_adv_fields fields = { 0 };
            int                      status = ble_hs_adv_parse_fields(&fields,
                                                 p_event->disc.data,
                                                 (p_event->disc).length_data);

            if (0 != status)
            {
                ESP_LOGE(BLE_TAG,
                         "Failed to parse advertisement data, error code: %d",
                         status);
                return 0;
            }

            if ((0 == fields.name_len) || (NULL == fields.name))
            {
                return 0;
            }

            if (NULL
                != strnstr((char *)fields.name, LEFT_LENS, fields.name_len))
            {
                ESP_LOGI(BLE_TAG,
                         "Discovered device: %.*s",
                         fields.name_len,
                         fields.name);

                g_is_connecting = true;
                ble_gap_disc_cancel();
                xEventGroupClearBits(g_pixel.p_ble_event_group, BLE_ALL);
                xEventGroupSetBits(g_pixel.p_ble_event_group, BLE_CONNECTING);

                struct ble_gap_conn_params conn_params
                    = { .scan_itvl           = 0x50,
                        .scan_window         = 0x30,
                        .itvl_min            = 0x10,
                        .itvl_max            = 0x20,
                        .latency             = 0,
                        .supervision_timeout = 1000,
                        .min_ce_len          = 0x10,
                        .max_ce_len          = 0x20 };

                // status = ble_gap_connect(p_event->disc.addr.type,
                status = ble_gap_connect(g_own_addr_type,
                                         &p_event->disc.addr,
                                         10000,
                                         &conn_params,
                                         ble_gap_event,
                                         NULL);

                if (0 != status)
                {
                    ESP_LOGE(BLE_TAG,
                             "Failed to connect to device, error code: %d",
                             status);
                    g_is_connecting = false;
                    ble_gap_disc(g_own_addr_type,
                                 BLE_HS_FOREVER,
                                 &g_scan_params,
                                 ble_gap_event,
                                 NULL);
                }
            }
        }
        break;

        case BLE_GAP_EVENT_DISC_COMPLETE: {
            ESP_LOGI(BLE_TAG,
                     "discovery complete, status=%d",
                     p_event->disc_complete.reason);
        }
        break;

        case BLE_GAP_EVENT_CONNECT: {
            g_is_connecting = false;

            if (0 == (p_event->connect).status)
            {
                ESP_LOGI(BLE_TAG, "Successfully connected!");
                xEventGroupClearBits(g_pixel.p_ble_event_group, BLE_ALL);
                xEventGroupSetBits(g_pixel.p_ble_event_group, BLE_CONNECTED);
            }
            else
            {
                ESP_LOGE(BLE_TAG,
                         "Connection failed, error: %d",
                         (p_event->connect).status);
                ble_gap_disc(g_own_addr_type,
                             BLE_HS_FOREVER,
                             &g_scan_params,
                             ble_gap_event,
                             NULL);
            }
        }
        break;

        case BLE_GAP_EVENT_DISCONNECT: {
            ESP_LOGI(BLE_TAG,
                     "disconnected, reason=%d",
                     p_event->disconnect.reason);
            xEventGroupClearBits(g_pixel.p_ble_event_group, BLE_ALL);
            xEventGroupSetBits(g_pixel.p_ble_event_group, BLE_SCANNING);
            ble_gap_disc(g_own_addr_type,
                         BLE_HS_FOREVER,
                         &g_scan_params,
                         ble_gap_event,
                         NULL);
        }
        break;

        case BLE_GAP_EVENT_CONN_UPDATE: {
            ESP_LOGI(BLE_TAG,
                     "connection updated, status=%d",
                     p_event->conn_update.status);
        }
        break;
        default: {
        }
        break;
    }

    return 0;
}

inline static void format_addr (char * addr_str, uint8_t addr[])
{
    sprintf(addr_str,
            "%02X:%02X:%02X:%02X:%02X:%02X",
            addr[0],
            addr[1],
            addr[2],
            addr[3],
            addr[4],
            addr[5]);
}

static void print_conn_desc (struct ble_gap_conn_desc * desc)
{
    char addr_str[18] = { 0 };

    /* Connection handle */
    ESP_LOGI(BLE_TAG, "connection handle: %d", desc->conn_handle);

    /* Local ID address */
    format_addr(addr_str, desc->our_id_addr.val);
    ESP_LOGI(BLE_TAG,
             "device id address: type=%d, value=%s",
             desc->our_id_addr.type,
             addr_str);

    /* Peer ID address */
    format_addr(addr_str, desc->peer_id_addr.val);
    ESP_LOGI(BLE_TAG,
             "peer id address: type=%d, value=%s",
             desc->peer_id_addr.type,
             addr_str);

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
