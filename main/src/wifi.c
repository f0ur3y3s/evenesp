
#include "wifi.h"
#define INTERNAL_WIFI_CONNECTED_BIT BIT0

static EventGroupHandle_t gh_wifi_egroup = NULL;

static void wifi_event_handler (void *           p_arg,
                                esp_event_base_t event_base,
                                int32_t          event_id,
                                void *           p_event_data);

void wifi_init (void)
{
    gh_wifi_egroup = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(WIFI_TAG, "Wi-Fi initialized in STA mode.");
}

// static void wifi_event_handler (void *           p_arg,
//                                 esp_event_base_t event_base,
//                                 int32_t          event_id,
//                                 void *           p_event_data)
// {
//     if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
//     {
//         uint8_t mac[6] = { 0 };
//         esp_wifi_get_mac(WIFI_IF_STA, mac);
//         ESP_LOGI(WIFI_TAG,
//                  "Wi-Fi started. MAC: %02X:%02X:%02X:%02X:%02X:%02X",
//                  mac[0],
//                  mac[1],
//                  mac[2],
//                  mac[3],
//                  mac[4],
//                  mac[5]);
//         esp_wifi_connect();
//     }
//     else if (event_base == WIFI_EVENT
//              && event_id == WIFI_EVENT_STA_DISCONNECTED)
//     {
//         esp_wifi_connect();
//         ESP_LOGI(WIFI_TAG, "Reconnecting to Wi-Fi...");
//     }
//     else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
//     {
//         ip_event_got_ip_t * event = (ip_event_got_ip_t *)p_event_data;
//         ESP_LOGI(WIFI_TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
//         xEventGroupSetBits(gh_wifi_egroup, INTERNAL_WIFI_CONNECTED_BIT);
//     }
// }

static void wifi_event_handler (void *           p_arg,
                                esp_event_base_t event_base,
                                int32_t          event_id,
                                void *           p_event_data)
{
    if (WIFI_EVENT == event_base)
    {
        if (WIFI_EVENT_STA_START == event_id)
        {
            ESP_LOGI(WIFI_TAG, "Wi-Fi started.");
        }
        else if (WIFI_EVENT_STA_DISCONNECTED == event_id)
        {
            ESP_LOGI(WIFI_TAG, "Reconnecting to Wi-Fi...");
        }
        else
        {
            ESP_LOGW(WIFI_TAG,
                     "NOT IMPLEMENTED Wi-Fi event: %" PRIi32,
                     event_id);
        }
        xEventGroupClearBits(g_pixel.p_ble_event_group, WIFI_ALL);
        xEventGroupSetBits(g_pixel.p_ble_event_group, WIFI_SCANNING);

        esp_wifi_connect();
    }
    else if (IP_EVENT == event_base)
    {
        if (IP_EVENT_STA_GOT_IP == event_id)
        {
            ip_event_got_ip_t * p_event = (ip_event_got_ip_t *)p_event_data;
            ESP_LOGI(WIFI_TAG,
                     "Got IP: " IPSTR,
                     IP2STR(&((p_event->ip_info).ip)));
            xEventGroupClearBits(g_pixel.p_ble_event_group, WIFI_ALL);
            xEventGroupSetBits(g_pixel.p_ble_event_group, WIFI_CONNECTED);
        }
        else
        {
            ESP_LOGW(WIFI_TAG, "NOT IMPLEMENTED IP event: %" PRIi32, event_id);
            xEventGroupClearBits(g_pixel.p_ble_event_group, WIFI_ALL);
        }
    }
    else
    {
        ESP_LOGW(WIFI_TAG, "NOT IMPLEMENTED event base: %s", event_base);
        xEventGroupClearBits(g_pixel.p_ble_event_group, WIFI_ALL);
    }
}