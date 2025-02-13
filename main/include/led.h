#ifndef EVEN_LED_H
#define EVEN_LED_H

#include <stdint.h>
#include "led_strip.h"
#include "esp_log.h"

/* Defaults for onboard neopixel */
#define LED_STRIP_GPIO 48
#define LED_STRIP_NUM  1
#define MAX_HUE        360
#define LED_TAG        "LED"

typedef enum group_bits_e
{
    BLE_SCANNING   = BIT0,
    BLE_CONNECTING = BIT1,
    BLE_CONNECTED  = BIT2,
    BLE_ALL        = (BIT0 | BIT1 | BIT2)
} group_bits_e;

typedef struct hsv_t
{
    uint16_t hue;
    uint8_t  saturation;
    uint8_t  value;
} hsv_t;

/**
 * @brief Structure to hold the onboard neopixel handle and event group
 *
 * @note Prevents reinitialization with bool
 *
 */
typedef struct pixel_t
{
    bool               is_initialized;
    EventGroupHandle_t p_ble_event_group;
    led_strip_handle_t p_led_strip;
} pixel_t;

/**
 * @brief A wrapper function to set hsv of the onboard neopixel
 *
 * @param p_led_strip Handle to the onboard neopixel
 * @param p_hsv Pointer to the hsv structure
 * @return esp_err_t
 * @retval ESP_OK On success
 * @retval ESP_FAIL On failure
 */
static inline esp_err_t led_set_hsv (led_strip_handle_t p_led_strip,
                                     hsv_t *            p_hsv)
{
    esp_err_t status = ESP_FAIL;

    if ((NULL == p_led_strip) || (NULL == p_hsv))
    {
        goto EXIT;
    }

    if (MAX_HUE <= p_hsv->hue)
    {
        p_hsv->hue -= MAX_HUE;
    }

    status = led_strip_set_pixel_hsv(p_led_strip,
                                     0,
                                     p_hsv->hue,
                                     p_hsv->saturation,
                                     p_hsv->value);

    if (ESP_OK == status)
    {
        status = led_strip_refresh(p_led_strip);
    }

EXIT:
    return status;
}

/**
 * @brief Initialize the onboard neopixel and creates a task for event group
 *
 */
void led_init ();

/**
 * @brief Extern to allow access to onboard neopixel by header
 *
 */
extern pixel_t g_pixel;

#endif // EVEN_LED_H