#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ClientRuntime.h"
#include "ParallelExecutor.h"

static double monotonic_seconds(void)
{
    struct timespec value;

    if (clock_gettime(CLOCK_MONOTONIC, &value) != 0) {
        return -1.0;
    }
    return (double)value.tv_sec + (double)value.tv_nsec / 1000000000.0;
}

static void make_input(int* values, size_t count, unsigned pattern)
{
    uint32_t state = 0x12345678UL;
    size_t index;

    for (index = 0; index < count; ++index) {
        state = state * 1664525UL + 1013904223UL;
        switch (pattern) {
        case 0: values[index] = (int)(state % 2000001) - 1000000; break;
        case 1: values[index] = (int)index; break;
        case 2: values[index] = (int)(count - index); break;
        default: values[index] = (int)(state % 17) - 8; break;
        }
    }
}

static int16_t benchmark_size(IEcoSortAndSearch* sort,
    ParallelPool* pool, size_t count, unsigned pattern)
{
    const char* patterns[] = {"random", "sorted", "reverse", "duplicates"};
    const char* methods[] = {"bitonic_1", "bitonic_4", "qsort"};
    int* input = (int*)malloc(count * sizeof(int));
    int* working = (int*)malloc(count * sizeof(int));
    int* expected = (int*)malloc(count * sizeof(int));
    unsigned repetition;
    unsigned offset;
    unsigned method;
    double begin;
    double elapsed;
    int16_t status = ERR_ECO_OUTOFMEMORY;

    if (input == 0 || working == 0 || expected == 0) {
        goto cleanup;
    }
    make_input(input, count, pattern);
    memcpy(expected, input, count * sizeof(int));
    qsort(expected, count, sizeof(int), compare_ints);
    for (repetition = 0; repetition <= 7; ++repetition) {
        for (offset = 0; offset < 3; ++offset) {
            method = (offset + repetition) % 3;
            memcpy(working, input, count * sizeof(int));
            if (method != 2) {
                status = sort->pVTbl->SetExecutor(sort, method == 1 ? 4 : 1,
                    method == 1 ? parallel_pool_execute : 0,
                    method == 1 ? pool : 0);
                if (status != 0) {
                    goto cleanup;
                }
            }
            begin = monotonic_seconds();
            if (begin < 0) {
                status = ERR_ECO_FAIL;
                goto cleanup;
            }
            if (method == 2) {
                qsort(working, count, sizeof(int), compare_ints);
            }
            else {
                sort->pVTbl->Sort(sort, working, count, sizeof(int), compare_ints);
            }
            elapsed = monotonic_seconds() - begin;
            if (method != 2 && sort->pVTbl->GetStatus(sort, 0) != 0) {
                status = ERR_ECO_FAIL;
                goto cleanup;
            }
            if (elapsed < 0 || memcmp(working, expected, count * sizeof(int)) != 0) {
                status = ERR_ECO_FAIL;
                goto cleanup;
            }
            if (repetition != 0) {
                printf("BENCH,%d,%lu,%s,%s,%u,%.9f,1\n", ARPO_LAB,
                    (unsigned long)count, patterns[pattern], methods[method],
                    repetition, elapsed);
            }
        }
    }
    status = ERR_ECO_SUCCESS;

cleanup:
    free(expected);
    free(working);
    free(input);
    return status;
}

int16_t ECOCALLMETHOD EcoMain(IEcoUnknown* system)
{
    const size_t sizes[] = {1024, 8192, 65536, 262144};
    ClientRuntime runtime;
    ParallelPool pool;
    IEcoSortAndSearch* sort = 0;
    size_t index;
    unsigned pattern;
    int16_t status;

    status = client_open(system, &runtime);
    if (status != 0) {
        return status;
    }
    status = runtime.bus->pVTbl->QueryComponent(runtime.bus,
#if ARPO_LAB == 1
        &CID_EcoLab1,
#else
        &CID_EcoLab2,
#endif
        0, &IID_IEcoSortAndSearch, (void**)&sort);
    if (status != 0 || sort == 0) {
        client_close(&runtime);
        return ERR_ECO_FAIL;
    }
    status = parallel_pool_open(&pool, 4);
    if (status == 0) {
        for (index = 0; index < sizeof(sizes) / sizeof(sizes[0]); ++index) {
            for (pattern = 0; pattern < 4; ++pattern) {
                status = benchmark_size(sort, &pool, sizes[index], pattern);
                if (status != 0) {
                    goto finished;
                }
            }
        }
finished:
        sort->pVTbl->SetExecutor(sort, 1, 0, 0);
        parallel_pool_close(&pool);
    }
    sort->pVTbl->Release(sort);
    client_close(&runtime);
    return status;
}
