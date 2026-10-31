#ifndef ARPO_I_ECO_SORT_AND_SEARCH_H
#define ARPO_I_ECO_SORT_AND_SEARCH_H

#include <stddef.h>
#include "IEcoBase1.h"
#include "LabIds.h"

typedef int (*EcoCompare)(const void* left, const void* right);
typedef void (ECOCALLMETHOD *EcoSortJob)(void* context,
    uint32_t lane, uint32_t lanes);

typedef int16_t (ECOCALLMETHOD *EcoSortExecute)(void* context,
    EcoSortJob job, void* job_context, uint32_t lanes);

typedef struct EcoSortStats {
    uint32_t padded_count;
    uint32_t stages;
    uint64_t network_pairs;
    uint64_t scratch_bytes;
} EcoSortStats;

typedef struct IEcoSortAndSearch IEcoSortAndSearch;
typedef struct IEcoSortAndSearchVTbl {
    int16_t (ECOCALLMETHOD *QueryInterface)(IEcoSortAndSearch* me,
        const UGUID* iid, void** result);
    uint32_t (ECOCALLMETHOD *AddRef)(IEcoSortAndSearch* me);
    uint32_t (ECOCALLMETHOD *Release)(IEcoSortAndSearch* me);

    void (ECOCALLMETHOD *Sort)(IEcoSortAndSearch* me, void* base,
        size_t count, size_t width, EcoCompare compare);
    int16_t (ECOCALLMETHOD *SetExecutor)(IEcoSortAndSearch* me,
        uint32_t lanes, EcoSortExecute execute, void* context);
    int16_t (ECOCALLMETHOD *GetStatus)(IEcoSortAndSearch* me,
        EcoSortStats* statistics);
} IEcoSortAndSearchVTbl;

struct IEcoSortAndSearch {
    IEcoSortAndSearchVTbl* pVTbl;
};

#endif
