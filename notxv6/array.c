#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

#define LARGE_NUMBER 10000
#define CHUNK 32
#define NTHREAD 2

struct producer
{
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int *job[CHUNK];
    int active_jobs;
} prd;

static void prod_init(void)
{
    assert(pthread_mutex_init(&prd.lock, NULL) == 0);
    assert(pthread_cond_init(&prd.cond, NULL) == 0);
    prd.active_jobs = 0;
}

static void *consumer(void *f)
{
    pthread_mutex_lock(&prd.lock);
    while (prd.active_jobs != -1)
    {
        if (prd.active_jobs > 0)
        {
            int *job = prd.job[prd.active_jobs - 1];
            prd.job[prd.active_jobs--] = 0;
            pthread_mutex_unlock(&prd.lock);
            ((void (*)(int *, int))f)(job, CHUNK);
            pthread_mutex_lock(&prd.lock);
        }
        else
        {
            pthread_cond_broadcast(&prd.cond);       // wake up producer
            pthread_cond_wait(&prd.cond, &prd.lock); // currently have no job, sleep
        }
    }
    pthread_mutex_unlock(&prd.lock);
    return 0;
}

static void fill_chunk(int *chunk, int size)
{
    for (int i = 0; i < size; ++i)
    {
        chunk[i] = rand() % 10;
    }
}

int main()
{
    int large_array[LARGE_NUMBER][CHUNK];
    prod_init();
    pthread_t *tha = malloc(sizeof(pthread_t) * NTHREAD);
    for (int thrd = 0; thrd < NTHREAD; ++thrd)
    {
        assert(pthread_create(&tha[thrd], NULL, consumer, fill_chunk) == 0);
    }

    for (int chunk_num = 0; chunk_num < LARGE_NUMBER; ++chunk_num)
    {
        pthread_mutex_lock(&prd.lock);

        if (prd.active_jobs == 32)
        {
            pthread_cond_broadcast(&prd.cond);
            pthread_cond_wait(&prd.cond, &prd.lock);
        }
        prd.job[prd.active_jobs] = &large_array[chunk_num][0];
        prd.active_jobs++;
        pthread_cond_broadcast(&prd.cond);
        pthread_mutex_unlock(&prd.lock);
    }

    pthread_mutex_lock(&prd.lock);
    while (prd.active_jobs != 0)
    {
        pthread_cond_wait(&prd.cond, &prd.lock);
    }
    prd.active_jobs = -1;
    pthread_cond_broadcast(&prd.cond);
    pthread_mutex_unlock(&prd.lock);

    void *value;
    for (int thrd = 0; thrd < NTHREAD; thrd++)
    {
        assert(pthread_join(tha[thrd], &value) == 0);
    }
    for (int i = 0; i < LARGE_NUMBER; i++)
    {
        printf("Number in [%d][0] : %d\n", i, large_array[i][0]);
    }
    return 0;
}