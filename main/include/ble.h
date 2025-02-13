#ifndef EVEN_BLE_H
#define EVEN_BLE_H

/* ESP APIs */
#include "sdkconfig.h"
#include "esp_log.h"

/* NimBLE stack APIs */
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/ble_gap.h"
#include "host/util/util.h"
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

/* custom headers */
#include "led.h"

#define BLE_TAG "ble"
// TODO: Change this to name of devices
// #define TARGET_DEVICE_NAME "NimBLE_CONN"
#define LEFT_LENS  "Even G1_1_L_"
#define RIGHT_LENS "Even G1_1_R_"

/**
 * @brief Initilizes the BLE stack with NimBLE host
 *
 * @return esp_err_t
 * @retval ESP_OK On success
 * @retval ESP_FAIL On failure
 */
esp_err_t ble_init (void);

/**
 * @brief xTask function to run the BLE host
 *
 * @param p_param Task parameters
 */
void ble_host_task (void * p_param);

#endif // EVEN_BLE_H