#ifndef ARPO_SORT_STATE_H
#define ARPO_SORT_STATE_H

#include "IEcoSortAndSearch.h"
#include "IEcoMemoryAllocator1.h"
#include "ErrEcoCodes.h"

#define ECO_SORT_MAX_LANES 64

typedef struct SortState {
    uint32_t lanes;
    EcoSortExecute execute;
    void* executor_context;
    int16_t last_status;
    EcoSortStats statistics;
} SortState;

void sort_state_initialize(SortState* state);
int16_t sort_state_set_executor(SortState* state, uint32_t lanes,
    EcoSortExecute execute, void* context);
void sort_state_run(SortState* state, IEcoMemoryAllocator1* memory,
    void* base, size_t count, size_t width, EcoCompare compare);
int16_t sort_state_status(const SortState* state, EcoSortStats* statistics);

#endif
