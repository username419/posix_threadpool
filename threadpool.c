#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "threadpool.h"

#define QUEUE_SIZE 1000
#define NUMBER_OF_THREADS 3

#define TRUE 1

// this represents work that has to be
// completed by a thread in the pool
typedef struct
{
    void (*function)(void *p);
    void *data;
}task;

// work queue
task worktodo[QUEUE_SIZE];

// the worker bee
pthread_t bee[NUMBER_OF_THREADS];
// mutex lock for the queue
pthread_mutex_t mutex;
// semaphore to signal that the queue is no longer empty
sem_t empty;
sem_t full;
int in=0;
int out=0;
int kill = 0;

// insert a task into the queue
// returns 0 if successful or 1 otherwise,
int enqueue(task t)
{
    sem_wait(&full);
    pthread_mutex_lock(&mutex);     // waits on full, to ensure that there is room, then gets mutex lock

    worktodo[in] = t;               // add task to position [in]
    in = (in+1) % QUEUE_SIZE;       // inc in
    printf("Added an task to the queue.\n");    // writeout

    pthread_mutex_unlock(&mutex);   // release mutex
    sem_post(&empty);                // signal that the queue has increased by 1

    return 0;
}

// remove a task from the queue
task dequeue()
{
    sem_wait(&empty);
    if(kill == 1)
    {
        return;
    }
    pthread_mutex_lock(&mutex); //waits on empty, to ensure that there is something to take, then gets mutex lock

    task *returnTask;
    returnTask =  (task *)malloc(sizeof(task));     // saves the (soon to be) dequeued task
    *returnTask = worktodo[out];
    out = (out+1)% QUEUE_SIZE;                      // increment out
    printf("Removed an task from the queue.\n");    // writeout

    pthread_mutex_unlock(&mutex);
    sem_post(&full);                        // signals that the queue has opened up a spot, release mutex

    return *returnTask;
}

// the worker thread in the thread pool
void *worker(void *param)
{
    while(TRUE)             // keeps looping as it waits for more jobs to pop up
    {
        task returnTask = dequeue();
        if(kill == 1)
        {
            break;
        }
            // execute the task
        execute(returnTask.function, returnTask.data);
    }

    pthread_exit(0);
}

/**
* Executes the task provided to the thread pool
*/
void execute(void (*somefunction)(void *p), void *p)
{
(*somefunction)(p);
}

/**
* Submits work to the pool.
*/
int pool_submit(void (*somefunction)(void *p), void *p)
{
    task *newTask = (task *)malloc(sizeof(task));
    (*newTask).function = somefunction;
    (*newTask).data = p;

    enqueue(*newTask);

    return 0;
}

// initialize the thread pool
void pool_init(void)
{
    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty,0,0);
    sem_init(&full,0,QUEUE_SIZE);               // initialize our semaphores/mutex

    for(int i = 0; i < NUMBER_OF_THREADS; i++)  // create the threads and lets them run free
    {
        pthread_create(&bee[i],NULL,worker,NULL);
    }
}

// shutdown the thread pool
void pool_shutdown(void)
{
    kill = 1;

    for(int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        sem_post(&empty);
        pthread_cancel(bee[i]);                 // tells all threads to wrap it up and come back for supper
    }

    for(int i = 0; i < NUMBER_OF_THREADS; i++)      // joins those threads
    {
            pthread_join(bee[i],NULL);
    }

    pthread_mutex_destroy(&mutex);                  // destroys semaphores/mutex
    sem_destroy(&empty);
    sem_destroy(&full);
    printf("Pool successfully shut down.\n");
}
