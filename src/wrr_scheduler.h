#ifndef WRR_SCHEDULER_H
#define WRR_SCHEDULER_H

#include <stddef.h>
#include <stdbool.h>

typedef enum {
    ALLOC_FAIL = 0,
    ALLOC_SUCCESS = 1
} alloc_outcome_t;

struct ue;

/* Allocation callback */
typedef alloc_outcome_t (*alloc_ue_fn)(struct ue *ue, void *ctx);

/* Weighted Round Robin scheduler entry point */
size_t wrr_schedule(struct ue *ues,
                    size_t ue_count,
                    size_t next_idx,
                    alloc_ue_fn alloc_fn,
                    void *ctx);

#endif
