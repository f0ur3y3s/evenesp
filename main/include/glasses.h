#ifndef EVEN_GLASSES_H
#define EVEN_GLASSES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct lens_t
{

} lens_t;

typedef struct glasses_t
{
    lens_t  left;
    lens_t  right;
    uint8_t seq;
    bool    received_ack;
} glasses_t;

#endif // EVEN_GLASSES_H