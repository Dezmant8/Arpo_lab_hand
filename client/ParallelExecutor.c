#include "ParallelExecutor.h"
#include "ErrEcoCodes.h"

static void* worker_main(void* argument)
{
    PoolWorker* worker = (PoolWorker*)argument;
    ParallelPool* pool = worker->pool;
    unsigned long observed = 0;
    EcoSortJob job;
    void* context;

    pthread_mutex_lock(&pool->mutex);
    for (;;) {
        while (!pool->stopping && pool->generation == observed) {
            pthread_cond_wait(&pool->work_ready, &pool->mutex);
        }
        if (pool->stopping) {
            break;
        }
        observed = pool->generation;
        job = pool->job;
        context = pool->job_context;
        pthread_mutex_unlock(&pool->mutex);
        job(context, worker->lane, pool->lanes);
        pthread_mutex_lock(&pool->mutex);
        if (--pool->pending == 0) {
            pthread_cond_signal(&pool->work_done);
        }
    }
    pthread_mutex_unlock(&pool->mutex);
    return 0;
}

int16_t parallel_pool_open(ParallelPool* pool, uint32_t lanes)
{
    uint32_t index;

    if (pool == 0 || lanes == 0 || lanes > ECO_SORT_MAX_LANES) {
        return ERR_ECO_INVALIDARG;
    }
    pool->lanes = lanes;
    pool->created = 0;
    pool->pending = 0;
    pool->generation = 0;
    pool->stopping = 0;
    pool->busy = 0;
    pool->job = 0;
    pool->job_context = 0;
    if (pthread_mutex_init(&pool->mutex, 0) != 0) {
        return ERR_ECO_FAIL;
    }
    if (pthread_cond_init(&pool->work_ready, 0) != 0) {
        pthread_mutex_destroy(&pool->mutex);
        return ERR_ECO_FAIL;
    }
    if (pthread_cond_init(&pool->work_done, 0) != 0) {
        pthread_cond_destroy(&pool->work_ready);
        pthread_mutex_destroy(&pool->mutex);
        return ERR_ECO_FAIL;
    }
    for (index = 0; index + 1 < lanes; ++index) {
        pool->workers[index].pool = pool;
        pool->workers[index].lane = index + 1;
        if (pthread_create(&pool->workers[index].thread, 0,
            worker_main, &pool->workers[index]) != 0) {
            parallel_pool_close(pool);
            return ERR_ECO_FAIL;
        }
        ++pool->created;
    }
    return ERR_ECO_SUCCESS;
}

void parallel_pool_close(ParallelPool* pool)
{
    uint32_t index;

    pthread_mutex_lock(&pool->mutex);
    pool->stopping = 1;
    pthread_cond_broadcast(&pool->work_ready);
    pthread_mutex_unlock(&pool->mutex);
    for (index = 0; index < pool->created; ++index) {
        pthread_join(pool->workers[index].thread, 0);
    }
    pthread_cond_destroy(&pool->work_done);
    pthread_cond_destroy(&pool->work_ready);
    pthread_mutex_destroy(&pool->mutex);
}

int16_t ECOCALLMETHOD parallel_pool_execute(void* context,
    EcoSortJob job, void* job_context, uint32_t lanes)
{
    ParallelPool* pool = (ParallelPool*)context;

    if (pool == 0 || job == 0 || lanes != pool->lanes) {
        return ERR_ECO_INVALIDARG;
    }
    pthread_mutex_lock(&pool->mutex);
    if (pool->busy || pool->stopping) {
        pthread_mutex_unlock(&pool->mutex);
        return ERR_ECO_FAIL;
    }
    pool->busy = 1;
    pool->job = job;
    pool->job_context = job_context;
    pool->pending = pool->created;
    ++pool->generation;
    pthread_cond_broadcast(&pool->work_ready);
    pthread_mutex_unlock(&pool->mutex);

    job(job_context, 0, lanes);

    pthread_mutex_lock(&pool->mutex);
    while (pool->pending != 0) {
        pthread_cond_wait(&pool->work_done, &pool->mutex);
    }
    pool->job = 0;
    pool->job_context = 0;
    pool->busy = 0;
    pthread_mutex_unlock(&pool->mutex);
    return ERR_ECO_SUCCESS;
}
