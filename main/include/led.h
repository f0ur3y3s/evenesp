#ifndef EVEN_LED_H
#define EVEN_LED_H

#include <stdint.h>
#include "led_strip.h"

// Defaults for onboard neopixel
#define LED_STRIP_GPIO 48
#define LED_STRIP_NUM  1
#define MAX_HUE        360

typedef struct hsv_t
{
    uint16_t hue;
    uint8_t  saturation;
    uint8_t  value;
} hsv_t;

static inline esp_err_t led_set_hsv (led_strip_handle_t h_led_strip, hsv_t * p_hsv)
{
    esp_err_t status = ESP_FAIL;

    if ((NULL == h_led_strip) || (NULL == p_hsv))
    {
        goto EXIT;
    }

    if (MAX_HUE <= p_hsv->hue)
    {
        p_hsv->hue -= MAX_HUE;
    }

    status = led_strip_set_pixel_hsv(h_led_strip, 0, p_hsv->hue, p_hsv->saturation, p_hsv->value);

    if (ESP_OK == status)
    {
        status = led_strip_refresh(h_led_strip);
    }

EXIT:
    return status;
}

void led_configure (led_strip_handle_t * pp_led_strip);

#endif // EVEN_LED_H