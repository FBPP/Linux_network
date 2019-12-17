#include "thread_pool.h"


#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define DEFAULT_TIME 10  /* 10s检测一次 */
#define MIN_WAIT_TASK_NUM 10  /* 如果queue_size > MIN_WAIT_TASK_NUM 添加新的线程到线程池 */
#define DEFAULT_THREAD_VARY 10  /* 每次创建和销毁线程的个数 */

#define true 1
#define false 0




thread_pool_t * thread_pool_create(int min_thr_num, int max_thr_num, int queue_max_size)
{
	int i;
	thread_pool_t *pool = NULL;
	do
	{
		if((pool = (thread_pool_t *)malloc(sizeof(thread_pool_t))) == NULL){
			printf("malloc thread_pool_t * fail");
			break;
		}

		pool->min_thr_num = min_thr_num;
		pool->max_thr_num = max_thr_num;
		pool->busy_thr_num = 0;
		pool->live_thr_num = min_thr_num;
		pool->queue_max_size = queue_max_size;
		pool->queue_size = 0;
		pool->queue_front = 0;
		pool->queue_rear = 0;
		pool->shutdown = false;

		pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * max_thr_num);
		if(pool->threads == NULL){
			printf("malloc threads fail");
			break;
		}
		memset(pool->threads, 0, sizeof(pthread_t) * max_thr_num);

		pool->task_queue = (thread_pool_task_t *)malloc(sizeof(thread_pool_task_t) * queue_max_size);
		if(pool->task_queue == NULL){
			printf("malloc thask_queue fail");
			break;
		}

		if(pthread_mutex_init(&(pool->lock), NULL) != 0
			|| pthread_mutex_init(&(pool->thread_counter), NULL) != 0
			|| pthread_cond_init(&(pool->queue_not_empty), NULL) != 0
			|| pthread_cond_init(&(pool->queue_not_full), NULL) != 0				
			) {
				printf("init the mutex or cond fail");
				break;
				}

		for( i = 0; i < min_thr_num; i++){
			pthread_create(&(pool->threads[i]), NULL, thread_pool_thread, (void *)pool);
			printf("start thread 0x%x...\n", (unsigned int)pool->threads[i]);
		}

		pthread_create(&(pool->adjust_tid), NULL, adjust_thread, (void *)pool);

		return pool;
		
	}while(0);

	thread_pool_free(pool);

	return NULL;
	
}

void * thread_pool_thread(void *thread_pool)
{
	thread_pool_t *pool = (thread_pool_t *)thread_pool;
	thread_pool_task_t task;

	while(true)
	{
		pthread_mutex_lock(&pool->lock);

		while( pool->queue_size == 0 && !pool->shutdown)
		{
				printf("thread0x%x is waiting\n", (unsigned int)pthread_self() );
				pthread_cond_wait(&pool->queue_not_empty, &pool->lock);

				if(pool->wait_exit_thr_num > 0)
				{
					--pool->wait_exit_thr_num;
					
				if(pool->live_thr_num > pool->min_thr_num){
					printf("thread 0x%x is exiting\n", (unsigned int)pthread_self() );
					--pool->live_thr_num;
					pthread_mutex_unlock(&pool->lock);
					pthread_exit(NULL);
					}
				}
		}

		if(pool->shutdown){
			pthread_mutex_unlock(&pool->lock);
			printf("thread 0x%x is exiting\n", (unsigned int)pthread_self() );
			pthread_exit(NULL);
		}

		task.function = pool->task_queue[pool->queue_front].function;
		task.arg = pool->task_queue[pool->queue_front].arg;

		pool->queue_front = (pool->queue_front + 1) % pool->queue_max_size;
		--pool->queue_size;
		
		pthread_cond_broadcast(&pool->queue_not_full);
		
		pthread_mutex_unlock(&pool->lock);

		/* 执行任务 */
		printf("thread 0x%xm start working\n", (unsigned int)pthread_self());
		pthread_mutex_lock(&pool->thread_counter);
		++pool->busy_thr_num;
		pthread_mutex_unlock(&pool->thread_counter);
		
		(*(task.function))(task.arg);

		/* 任务结束 */
		printf("thread 0x%x end working\n", (unsigned int)pthread_self());
		pthread_mutex_lock(&pool->thread_counter);
		--pool->busy_thr_num;
		pthread_mutex_unlock(&pool->thread_counter);
		
			
	}

	pthread_exit(NULL);
}

void * adjust_thread(void *thread_pool)
{
	int i;
	thread_pool_t *pool = (thread_pool_t *)thread_pool;

	while(!pool->shutdown)
		{
			sleep(DEFAULT_TIME);

			pthread_mutex_lock(&pool->lock);
			int queue_size = pool->queue_size;
			int live_thr_num = pool->live_thr_num;
			int  busy_thr_num = pool->busy_thr_num;
			pthread_mutex_unlock(&pool->lock);

			if(queue_size >= MIN_WAIT_TASK_NUM && live_thr_num < pool->max_thr_num)
				{
					pthread_mutex_lock(&pool->lock);
					int add = 0;

					for(i = 0; i < pool->max_thr_num && add < DEFAULT_THREAD_VARY && pool->live_thr_num < pool->max_thr_num; i++)
							if(pool->threads[i] == 0 || !is_thread_alive(pool->threads[i])){
									pthread_create(&pool->threads[i], NULL, thread_pool_thread, (void *)pool);
									++add;
									++pool->live_thr_num;
								}
					pthread_mutex_unlock(&pool->lock);
				}

			if( (busy_thr_num *2) < live_thr_num && live_thr_num > pool->min_thr_num){
					pthread_mutex_lock(&pool->lock);
					pool->wait_exit_thr_num = DEFAULT_THREAD_VARY;
					pthread_mutex_unlock(&pool->lock);

					for(i = 0; i < DEFAULT_THREAD_VARY; i++)
							pthread_cond_signal(&pool->queue_not_empty);
				}

			

			
		}
	
}

int thread_pool_add(thread_pool_t *pool, void *(*function)(void *arg), void *arg)
{
	pthread_mutex_lock(&pool->lock);
	while(pool->queue_size == pool->queue_max_size && !pool->shutdown){
		pthread_cond_wait(&pool->queue_not_full, &pool->lock);	
	}

	if(pool->shutdown){
		pthread_mutex_unlock(&pool->lock);
	}

	pool->task_queue[pool->queue_rear].function = function;
	pool->task_queue[pool->queue_rear].arg = arg;
	pool->queue_rear = (pool->queue_rear + 1) % pool->queue_max_size;
	pool->queue_size++;

	pthread_cond_signal(&pool->queue_not_empty);
	//printf("%d process add\n",(int)arg);
	pthread_mutex_unlock(&pool->lock);

	return 0;
}

int thread_pool_destory(thread_pool_t *pool)
{
	int i;
	if(pool == NULL)
		return -1;

	pool->shutdown = true;

	pthread_join(pool->adjust_tid, NULL);

	for(i = 0; i < pool->live_thr_num; i++)
		pthread_cond_broadcast(&pool->queue_not_empty);
	for(i = 0; i < pool->live_thr_num; i++)
		pthread_join(pool->threads[i],NULL);
	thread_pool_free(pool);

	return 0;
}
int thread_pool_free(thread_pool_t *pool)
{
	if(pool == NULL)
		return -1;
	if(pool->task_queue)
		free(pool->task_queue);
	if(pool->threads){
		free(pool->threads);
		pthread_mutex_lock(&pool->lock);
		pthread_mutex_destroy(&pool->lock);
		pthread_mutex_lock(&pool->thread_counter);
		pthread_mutex_destroy(&pool->thread_counter);
		pthread_cond_destroy(&pool->queue_not_full);
		pthread_cond_destroy(&pool->queue_not_empty);
	}

	free(pool);
	pool = NULL;
	return 0;
}

int is_thread_alive(pthread_t tid)
{
	int kill_rc = pthread_kill(tid, 0);
	if(kill_rc == ESRCH)
		return false;
	return true;
}

#if 1

void * process(void *arg)
{
	printf("thread 0x%x working on task %d\n", (unsigned int)pthread_self(), (int)arg);
	sleep(1);
	printf("task %d is end\n", (int)arg);

	return NULL;
}

int main()
{
	thread_pool_t *thp = thread_pool_create(3, 100, 100);
	printf("pool inited");

	int num[20], i;
	for(i = 0; i < 20; i++){
		num[i] = i;
		printf("add task %d\n", i);
		thread_pool_add(thp, process, (void *)num[i]);
	}
	
	sleep(10);
	thread_pool_destory(thp);
	
	return 0;
}


#endif