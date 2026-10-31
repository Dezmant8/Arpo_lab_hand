#ifndef ARPO_PARALLEL_EXECUTOR_H
#define ARPO_PARALLEL_EXECUTOR_H

#include <pthread.h>
#include "IEcoSortAndSearch.h"
#include "SortState.h"

typedef struct ParallelPool ParallelPool;
typedef struct PoolWorker {
    ParallelPool* pool;
    uint32_t lane;
    pthread_t thread;
} PoolWorker;

struct ParallelPool {
    pthread_mutex_t mutex;
    pthread_cond_t work_ready;
    pthread_cond_t work_done;
    PoolWorker workers[ECO_SORT_MAX_LANES - 1];
    uint32_t lanes;
    uint32_t created;
    uint32_t pending;
    unsigned long generation;
    int stopping;
    int busy;
    EcoSortJob job;
    void* job_context;
};

int16_t parallel_pool_open(ParallelPool* pool, uint32_t lanes);
void parallel_pool_close(ParallelPool* pool);
int16_t ECOCALLMETHOD parallel_pool_execute(void* context,
    EcoSortJob job, void* job_context, uint32_t lanes);

#endif
