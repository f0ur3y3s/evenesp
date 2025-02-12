#ifndef EVEN_BLE_H
#define EVEN_BLE_H

/* ESP APIs */
#include "sdkconfig.h"
#include "esp_log.h"

/* NimBLE stack APIs */
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

// custom headers
#include "led.h"

#define BLE_TAG "ble"

esp_err_t ble_init (pixel_t * p_pixel);
void      ble_host_config_init (void);
void      ble_host_task (void * p_param);

#endif // EVEN_BLE_H