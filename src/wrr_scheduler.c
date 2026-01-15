/*
 * ByteBalancer - Weighted Round Robin (WRR) Scheduler (Reference C Sample)
 *
 * This is a small, readable WRR implementation you can include in your repo as
 * an algorithm reference (separate from any specific srsRAN integration).
 *
 * Core idea:
 * - Maintain a circular pointer over "active" UEs.
 * - For each UE, attempt to allocate resources up to `weight` times per round.
 * - Update the next starting UE after the last successful allocation.
 *
 * Notes:
 * - In real MAC schedulers, alloc_ue() would check buffer status, CQI/MCS,
 *   available RBs, HARQ constraints, etc. Here we model it with a callback.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_UES 64

typedef struct {
    uint16_t ue_id;      // can represent RNTI / UE index
    uint8_t  weight;     // WRR weight (>= 1)
    bool     active;     // eligible for scheduling
    // You can extend with: buffer_bytes, cqi, qos_class, etc.
} ue_t;

typedef enum {
    ALLOC_FAIL = 0,
    ALLOC_SUCCESS = 1
} alloc_outcome_t;

/*
 * Callback signature: attempt to allocate resources for a UE.
 * Return ALLOC_SUCCESS if the UE actually gets resources in this attempt.
 */
typedef alloc_outcome_t (*alloc_ue_fn)(const ue_t *ue, void *ctx);

/*
 * Weighted Round Robin scheduler:
 * - ues: array of UE contexts
 * - n: number of entries in ues[]
 * - next_idx: pointer to where we start scanning (circular)
 * - alloc_fn: allocation callback
 * - ctx: user context passed to alloc_fn
 *
 * Returns: updated next_idx for the next slot/TTI.
 */
size_t wrr_schedule(ue_t *ues,
                    size_t n,
                    size_t next_idx,
                    alloc_ue_fn alloc_fn,
                    void *ctx)
{
    if (n == 0 || alloc_fn == NULL) {
        return 0;
    }

    // Count active UEs. If none, keep pointer stable.
    size_t active_count = 0;
    for (size_t i = 0; i < n; i++) {
        if (ues[i].active && ues[i].weight > 0) {
            active_count++;
        }
    }
    if (active_count == 0) {
        return next_idx % n;
    }

    bool any_success = false;
    size_t idx = next_idx % n;

    // One full pass over all UEs (like RR), but each UE gets `weight` attempts.
    for (size_t visited = 0; visited < n; visited++) {
        ue_t *ue = &ues[idx];

        if (ue->active && ue->weight > 0) {
            uint8_t w = ue->weight;

            for (uint8_t k = 0; k < w; k++) {
                alloc_outcome_t res = alloc_fn(ue, ctx);
                if (res == ALLOC_SUCCESS) {
                    any_success = true;
                    // In many MAC schedulers, you may break after success
                    // if allocation is "one grant per slot". But if the system
                    // supports multiple allocations, keeping the loop is fine.
                }
            }

            // If at least one allocation succeeded in this round,
            // advance the pointer to the UE after the last UE we touched.
            if (any_success) {
                next_idx = (idx + 1) % n;
            }
        }

        idx = (idx + 1) % n;
    }

    return next_idx % n;
}

/* ------------------------- Demo / Example Usage ------------------------- */

typedef struct {
    // example: remaining resource "budget" for this scheduling instant
    int rb_budget;
} demo_ctx_t;

/*
 * Example allocator:
 * - Pretends each successful allocation consumes 1 RB unit.
 * - Only "allocates" if rb_budget > 0.
 */
static alloc_outcome_t demo_alloc_fn(const ue_t *ue, void *ctx)
{
    demo_ctx_t *d = (demo_ctx_t *)ctx;
    if (d->rb_budget <= 0) {
        return ALLOC_FAIL;
    }

    // For demo, allocate to active UE and burn budget.
    d->rb_budget--;

    printf("  allocated -> UE %u (weight=%u), remaining_budget=%d\n",
           ue->ue_id, ue->weight, d->rb_budget);

    return ALLOC_SUCCESS;
}

int main(void)
{
    ue_t ues[] = {
        { .ue_id = 1001, .weight = 1, .active = true  }, // low
        { .ue_id = 1002, .weight = 2, .active = true  }, // medium
        { .ue_id = 1003, .weight = 4, .active = true  }  // high
    };
    size_t n = sizeof(ues) / sizeof(ues[0]);

    size_t next_idx = 0;

    // Simulate multiple scheduling instants (slots/TTIs)
    for (int tti = 0; tti < 5; tti++) {
        demo_ctx_t ctx = { .rb_budget = 6 }; // pretend we have 6 RB units this instant

        printf("\nTTI %d (start_idx=%zu, rb_budget=%d)\n", tti, next_idx, ctx.rb_budget);
        next_idx = wrr_schedule(ues, n, next_idx, demo_alloc_fn, &ctx);
        printf("TTI %d done -> next_idx=%zu\n", tti, next_idx);
    }

    return 0;
}
