#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "threadpool.h"

struct data
{
    int a;
    int b;
};

void add(void *param)
{
    struct data *temp;
    temp = (struct data*)param;

    printf("I add two values %d and %d result = %d\n",temp->a, temp->b, temp->a + temp->b);
}

void sub(void *param)
{
    struct data *temp;
    temp = (struct data*)param;

    printf("I subtract two values %d and %d result = %d\n",temp->a, temp->b, temp->a - temp->b);
}

void mul(void *param)
{
    struct data *temp;
    temp = (struct data*)param;

    printf("I multiply two values %d and %d result = %d\n",temp->a, temp->b, temp->a * temp->b);
}

int main(int argc, char* argv[])
{
    // initialize the thread pool
    pool_init();

    // reads from command line the number of jobs to create
    int jobs = 11;

    if(argc > 1)
    {
        char *a = argv[1];
        jobs = atoi(a);
        printf("Total jobs counted: %d\n", jobs);
    }

    // creates those jobs, increments these two counters to simulate actually accomplishing something
    int counter1 = 5;
    int counter2 = 10;
    for(int i = 0; i < jobs; i++)
    {
        printf("Job %d: ",i);
        struct data *jobs = (struct data *)malloc(sizeof(struct data));
        jobs->a = counter1;
        jobs->b = counter2;
        pool_submit(&add,jobs);
        pool_submit(&sub,jobs);
        pool_submit(&mul,jobs);
        counter1++;
        counter2++;
    }

    // inelegant way to handle returning the threads created later.
    // simply times out, then moves on to pool_shutdown() which cancels them.
    sleep(1);

    pool_shutdown();

    return 0;
}
