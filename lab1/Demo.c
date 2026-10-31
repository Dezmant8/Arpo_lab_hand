#include <stdio.h>
#include "ClientRuntime.h"
#include "ParallelExecutor.h"

int16_t ECOCALLMETHOD EcoMain(IEcoUnknown* system)
{
    ClientRuntime runtime;
    IEcoSortAndSearch* sort = 0;
    ParallelPool pool;
    EcoSortStats statistics;
    int values[] = {18, -4, 7, 7, 0, 93, 1, -20, 6};
    size_t count = sizeof(values) / sizeof(values[0]);
    size_t index;
    int16_t status;

    status = client_open(system, &runtime);
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    status = runtime.bus->pVTbl->QueryComponent(runtime.bus, &CID_EcoLab1,
        0, &IID_IEcoSortAndSearch, (void**)&sort);
    if (status != ERR_ECO_SUCCESS || sort == 0) {
        client_close(&runtime);
        return ERR_ECO_FAIL;
    }
    status = parallel_pool_open(&pool, 4);
    if (status == ERR_ECO_SUCCESS) {
        status = sort->pVTbl->SetExecutor(sort, 4, parallel_pool_execute, &pool);
        if (status == ERR_ECO_SUCCESS) {
            sort->pVTbl->Sort(sort, values, count, sizeof(values[0]), compare_ints);
            status = sort->pVTbl->GetStatus(sort, &statistics);
        }
        sort->pVTbl->SetExecutor(sort, 1, 0, 0);
        parallel_pool_close(&pool);
    }
    if (status == ERR_ECO_SUCCESS) {
        printf("Lab1, bitonic sort, four lanes:\n");
        for (index = 0; index < count; ++index) {
            printf("%d%s", values[index], index + 1 == count ? "\n" : " ");
        }
        printf("padded=%lu stages=%lu network_pairs=%lu scratch_bytes=%lu\n",
            (unsigned long)statistics.padded_count,
            (unsigned long)statistics.stages,
            (unsigned long)statistics.network_pairs,
            (unsigned long)statistics.scratch_bytes);
    }
    sort->pVTbl->Release(sort);
    client_close(&runtime);
    return status;
}
