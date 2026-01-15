#ifndef UE_H
#define UE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct ue {
    uint16_t ue_id;     /* UE identifier (RNTI / index) */
    uint8_t  weight;   /* WRR weight */
    bool     active;   /* Eligible for scheduling */
} ue_t;

#endif
