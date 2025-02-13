#ifndef EVEN_WIFI_H
#define EVEN_WIFI_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <inttypes.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"

#include "wifi_secret.h"
#include "led.h"

#define WIFI_TAG "WIFI"

void wifi_init (void);

#endif // EVEN_WIFI_H