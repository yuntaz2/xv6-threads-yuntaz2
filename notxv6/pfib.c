#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

void *pong(void *arg)
{
    for (int i = 0; i < 10; ++i)
    {
        usleep(1000);
        printf("1");
    }

    return NULL;
}

void ping()
{
    pthread_t pthread;
    pthread_create(&pthread, NULL, pong, NULL);

    for (int i = 0; i < 10; ++i)
    {
        usleep(1000);
        printf("0");
    }
    pthread_join(pthread, NULL);
    printf("boom\n");

    return;
}

void *p_fib(void *arg)
{
    int *p = (int *)arg;
    printf("arg: %d\n", *p);
    int *res = (int *)malloc(sizeof(int));
    if (*p < 2)
    {
        *res = 1;
        return res;
    }
    pthread_t pthread_1;
    pthread_t pthread_2;
    int arg1 = *p - 1;
    int arg2 = *p - 2;
    pthread_create(&pthread_1, NULL, p_fib, (void **)&arg1); // spawn thread1
    pthread_create(&pthread_2, NULL, p_fib, (void **)&arg2); // spawn thread2

    // sync
    int *res1;
    int *res2;
    pthread_join(pthread_1, (void **)&res1);
    pthread_join(pthread_2, (void **)&res2);
    *res = (*res1 + *res2);
    free(res1);
    free(res2);
    return res;
}

int main(void)
{
    pthread_t pthread;
    int arg = 5;
    pthread_create(&pthread, NULL, p_fib, (void *)&arg);
    int *res;
    pthread_join(pthread, (void **)&res);
    printf("res: %d\n", *res);
    free(res);
    return 0;
}