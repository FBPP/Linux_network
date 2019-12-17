#ifndef THREAD_POOL_H
#define THREAD_POOL_H


#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

typedef struct
{
	void * (*function)(void *);
	void *arg;
} thread_pool_task_t;

typedef struct 
{
	pthread_mutex_t lock;  /* 用于锁住本结构体 */
	pthread_mutex_t thread_counter;  /* 记录工作中的线程的锁 */

	pthread_cond_t queue_not_full;
	pthread_cond_t queue_not_empty;

	pthread_t *threads; /* 指向数组的指针, 数组存放线程池中每个线程的tid */
	pthread_t adjust_tid;

	thread_pool_task_t *task_queue; /* 任务队列 */

	int min_thr_num;
	int max_thr_num;
	int live_thr_num;
	int busy_thr_num;
	int wait_exit_thr_num; /* 要销毁的线程数 */

	int queue_front;
	int queue_rear; 
	int queue_size;
	int queue_max_size;

	int shutdown;
	
}thread_pool_t;

thread_pool_t * thread_pool_create(int, int, int);
void * thread_pool_thread(void *);
void * adjust_thread(void *);
int thread_pool_add(thread_pool_t *, void *(*)(void *), void *);
int thread_pool_destory(thread_pool_t *);
int thread_pool_free(thread_pool_t *);
int is_thread_alive(pthread_t );

#endif
