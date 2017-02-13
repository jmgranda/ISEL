#ifndef TASK_H
#define TASK_H

#include <pthread.h>
#include <time.h>
#include <sys/time.h>

void task_setup (void);

pthread_t task_new (const char* name, void *(*f)(void *),
                    int period_ms, int deadline_ms,
                    int prio, int stacksize);
struct timespec *task_get_period (pthread_t tid);
struct timespec *task_get_deadline (pthread_t tid);

void mutex_init (pthread_mutex_t* m, int prioceiling);

void timeval_sub (struct timeval *res, struct timeval *a, struct timeval *b);
void timeval_add (struct timeval *res, struct timeval *a, struct timeval *b);
int timeval_less (struct timeval *a, struct timeval *b);
int timeval_get_ms (struct timeval *a);

void timespec_sub (struct timespec *res, struct timespec *a, struct timespec *b);
void timespec_add (struct timespec *res, struct timespec *a, struct timespec *b);
int timespec_less (struct timespec *a, struct timespec *b);

#endif

