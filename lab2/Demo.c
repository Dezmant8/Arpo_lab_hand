#include <stdio.h>
#include "ClientRuntime.h"
#include "ParallelExecutor.h"
#include "IEcoCalculatorX.h"
#include "IEcoCalculatorY.h"
#include "IEcoLab2.h"

static int16_t demonstrate(IEcoInterfaceBus1* bus, const UGUID* cid,
    const char* name, ParallelPool* pool)
{
    IEcoSortAndSearch* sort = 0;
    IEcoCalculatorX* calc_x = 0;
    IEcoCalculatorY* calc_y = 0;
    IEcoUnknown* from_sort = 0;
    IEcoUnknown* from_x = 0;
    IEcoUnknown* from_y = 0;
    int values[] = {9, -1, 3, 3, 0};
    int16_t status;

    status = bus->pVTbl->QueryComponent(bus, cid, 0,
        &IID_IEcoSortAndSearch, (void**)&sort);
    if (status != ERR_ECO_SUCCESS) {
        goto cleanup;
    }
    status = sort->pVTbl->QueryInterface(sort, &IID_IEcoCalculatorX,
        (void**)&calc_x);
    if (status != ERR_ECO_SUCCESS) {
        goto cleanup;
    }
    status = calc_x->pVTbl->QueryInterface(calc_x, &IID_IEcoCalculatorY,
        (void**)&calc_y);
    if (status != ERR_ECO_SUCCESS) {
        goto cleanup;
    }
    status = sort->pVTbl->QueryInterface(sort, &IID_IEcoUnknown,
        (void**)&from_sort);
    if (status != ERR_ECO_SUCCESS) {
        goto cleanup;
    }
    status = calc_x->pVTbl->QueryInterface(calc_x, &IID_IEcoUnknown,
        (void**)&from_x);
    if (status != ERR_ECO_SUCCESS) {
        goto cleanup;
    }
    status = calc_y->pVTbl->QueryInterface(calc_y, &IID_IEcoUnknown,
        (void**)&from_y);
    if (status != ERR_ECO_SUCCESS) {
        goto cleanup;
    }
    if (from_sort != from_x || from_x != from_y) {
        status = ERR_ECO_FAIL;
        goto cleanup;
    }
    status = sort->pVTbl->SetExecutor(sort, pool->lanes,
        parallel_pool_execute, pool);
    if (status != ERR_ECO_SUCCESS) {
        goto cleanup;
    }
    sort->pVTbl->Sort(sort, values, 5, sizeof(values[0]), compare_ints);
    status = sort->pVTbl->GetStatus(sort, 0);
    if (status == ERR_ECO_SUCCESS) {
        printf("%s: addition=%ld subtraction=%d multiplication=%ld division=%d\n",
            name, (long)calc_x->pVTbl->Addition(calc_x, 3, 5),
            (int)calc_x->pVTbl->Subtraction(calc_x, 3, 5),
            (long)calc_y->pVTbl->Multiplication(calc_y, 3, 5),
            (int)calc_y->pVTbl->Division(calc_y, 15, 3));
        printf("  canonical Unknown: same; sort: %d %d %d %d %d\n",
            values[0], values[1], values[2], values[3], values[4]);
    }

cleanup:
    if (from_y != 0) {
        from_y->pVTbl->Release(from_y);
    }
    if (from_x != 0) {
        from_x->pVTbl->Release(from_x);
    }
    if (from_sort != 0) {
        from_sort->pVTbl->Release(from_sort);
    }
    if (calc_y != 0) {
        calc_y->pVTbl->Release(calc_y);
    }
    if (calc_x != 0) {
        calc_x->pVTbl->Release(calc_x);
    }
    if (sort != 0) {
        sort->pVTbl->Release(sort);
    }
    return status;
}

int16_t ECOCALLMETHOD EcoMain(IEcoUnknown* system)
{
    const UGUID* ids[] = {&CID_Lab1IncludedA, &CID_Lab1AggregatedB,
        &CID_Lab1IncludedC, &CID_Lab1IncludedD, &CID_Lab1IncludedE, &CID_EcoLab2};
    const char* names[] = {"Lab1 includes A,D", "Lab1 aggregates B, includes D",
        "Lab1 includes C", "Lab1 includes D", "Lab1 includes E", "Lab2"};
    ClientRuntime runtime;
    ParallelPool pool;
    uint32_t index;
    int16_t status;

    status = client_open(system, &runtime);
    if (status != ERR_ECO_SUCCESS) {
        return status;
    }
    status = parallel_pool_open(&pool, 4);
    if (status == ERR_ECO_SUCCESS) {
        for (index = 0; index < sizeof(ids) / sizeof(ids[0]); ++index) {
            status = demonstrate(runtime.bus, ids[index], names[index], &pool);
            if (status != ERR_ECO_SUCCESS) {
                break;
            }
        }
        parallel_pool_close(&pool);
    }
    client_close(&runtime);
    return status;
}
