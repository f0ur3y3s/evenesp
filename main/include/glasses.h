#ifndef EVEN_GLASSES_H
#define EVEN_GLASSES_H

#include <stdint.h>
#include <stdbool.h>

#include "ble.h"

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

int glasses_init (void);
int glasses_deinit (void);
int glasses_connect (void);
int glasses_disconnect (void);
int glasses_send (void);

#endif // EVEN_GLASSES_H