#include "SortState.h"

#define PADDING_INDEX ((uint32_t)0xFFFFFFFFUL)
#define ALLOCATOR_LIMIT ((size_t)0xFFFFFFFFUL)

typedef struct BitonicStage {
    const unsigned char* input;
    uint32_t* order;
    size_t width;
    EcoCompare compare;
    uint32_t padded_count;
    uint32_t span;
    uint32_t stride;
} BitonicStage;

static void clear_statistics(EcoSortStats* statistics)
{
    statistics->padded_count = 0;
    statistics->stages = 0;
    statistics->network_pairs = 0;
    statistics->scratch_bytes = 0;
}

static void copy_bytes(unsigned char* destination,
    const unsigned char* source, size_t count)
{
    size_t index;

    for (index = 0; index < count; ++index) {
        destination[index] = source[index];
    }
}

static int compare_indices(const BitonicStage* stage,
    uint32_t left, uint32_t right)
{
    int order;

    if (left == right) {
        return 0;
    }
    if (left == PADDING_INDEX) {
        return 1;
    }
    if (right == PADDING_INDEX) {
        return -1;
    }
    order = stage->compare(stage->input + (size_t)left * stage->width,
        stage->input + (size_t)right * stage->width);
    if (order != 0) {
        return order;
    }
    return left < right ? -1 : 1;
}

static void ECOCALLMETHOD compare_stage(void* context,
    uint32_t lane, uint32_t lanes)
{
    BitonicStage* stage = (BitonicStage*)context;
    uint32_t pair;
    uint32_t left;
    uint32_t right;
    uint32_t temporary;
    int comparison;
    int ascending;

    for (pair = lane; pair < stage->padded_count / 2; pair += lanes) {
        left = (pair / stage->stride) * (2 * stage->stride)
            + pair % stage->stride;
        right = left + stage->stride;
        comparison = compare_indices(stage, stage->order[left],
            stage->order[right]);
        ascending = (left & stage->span) == 0;
        if ((ascending && comparison > 0) || (!ascending && comparison < 0)) {
            temporary = stage->order[left];
            stage->order[left] = stage->order[right];
            stage->order[right] = temporary;
        }
    }
}

static int16_t run_network(BitonicStage* stage, const SortState* state,
    EcoSortStats* statistics)
{
    int16_t status;

    for (stage->span = 2; stage->span <= stage->padded_count;
        stage->span *= 2) {
        for (stage->stride = stage->span / 2; stage->stride != 0;
            stage->stride /= 2) {
            if (state->lanes == 1) {
                compare_stage(stage, 0, 1);
            }
            else {
                status = state->execute(state->executor_context,
                    compare_stage, stage, state->lanes);
                if (status != ERR_ECO_SUCCESS) {
                    return status;
                }
            }
            ++statistics->stages;
            statistics->network_pairs += stage->padded_count / 2;
        }
    }
    return ERR_ECO_SUCCESS;
}

static int16_t bitonic_sort(const SortState* state,
    IEcoMemoryAllocator1* memory, void* base, size_t count, size_t width,
    EcoCompare compare, EcoSortStats* statistics)
{
    BitonicStage stage;
    unsigned char* sorted;
    uint32_t index;
    uint32_t padded = 1;
    size_t byte_count;
    int16_t status;

    clear_statistics(statistics);
    if (count == 0) {
        return ERR_ECO_SUCCESS;
    }
    if (base == 0 || compare == 0 || width == 0) {
        return ERR_ECO_INVALIDARG;
    }
    if (count > ALLOCATOR_LIMIT / width) {
        return ERR_ECO_INVALIDARG;
    }
    while ((size_t)padded < count) {
        if (padded > (uint32_t)(ALLOCATOR_LIMIT / (2 * sizeof(uint32_t)))) {
            return ERR_ECO_INVALIDARG;
        }
        padded *= 2;
    }
    statistics->padded_count = padded;
    if (count == 1) {
        return ERR_ECO_SUCCESS;
    }
    byte_count = count * width;
    stage.order = (uint32_t*)memory->pVTbl->Alloc(memory,
        padded * (uint32_t)sizeof(uint32_t));
    if (stage.order == 0) {
        return ERR_ECO_OUTOFMEMORY;
    }
    sorted = (unsigned char*)memory->pVTbl->Alloc(memory, (uint32_t)byte_count);
    if (sorted == 0) {
        memory->pVTbl->Free(memory, stage.order);
        return ERR_ECO_OUTOFMEMORY;
    }
    statistics->scratch_bytes = (uint64_t)padded * sizeof(uint32_t)
        + byte_count;
    stage.input = (const unsigned char*)base;
    stage.width = width;
    stage.compare = compare;
    stage.padded_count = padded;
    for (index = 0; index < padded; ++index) {
        stage.order[index] = index < count ? index : PADDING_INDEX;
    }
    status = run_network(&stage, state, statistics);
    if (status == ERR_ECO_SUCCESS) {
        for (index = 0; index < count; ++index) {
            copy_bytes(sorted + (size_t)index * width,
                stage.input + (size_t)stage.order[index] * width, width);
        }
        copy_bytes((unsigned char*)base, sorted, byte_count);
    }
    memory->pVTbl->Free(memory, sorted);
    memory->pVTbl->Free(memory, stage.order);
    return status;
}

void sort_state_initialize(SortState* state)
{
    state->lanes = 1;
    state->execute = 0;
    state->executor_context = 0;
    state->last_status = ERR_ECO_SUCCESS;
    clear_statistics(&state->statistics);
}

int16_t sort_state_set_executor(SortState* state, uint32_t lanes,
    EcoSortExecute execute, void* context)
{
    if (lanes == 0 || lanes > ECO_SORT_MAX_LANES ||
        (lanes > 1 && execute == 0)) {
        return ERR_ECO_INVALIDARG;
    }
    state->lanes = lanes;
    state->execute = execute;
    state->executor_context = context;
    return ERR_ECO_SUCCESS;
}

void sort_state_run(SortState* state, IEcoMemoryAllocator1* memory,
    void* base, size_t count, size_t width, EcoCompare compare)
{
    state->last_status = bitonic_sort(state, memory, base, count, width,
        compare, &state->statistics);
}

int16_t sort_state_status(const SortState* state, EcoSortStats* statistics)
{
    if (statistics != 0) {
        *statistics = state->statistics;
    }
    return state->last_status;
}
