#ifndef EVEN_GLASSES_H
#define EVEN_GLASSES_H

#include <stdint.h>
#include <stdbool.h>
#include "host/ble_hs.h"

typedef struct lens_t
{
    bool              is_connecting;
    bool              is_connected;
    ble_hs_adv_fields fields;
} lens_t;

typedef struct glasses_t
{
    lens_t  left;
    lens_t  right;
    uint8_t seq;
    bool    received_ack;
} glasses_t;

#endif // EVEN_GLASSES_H