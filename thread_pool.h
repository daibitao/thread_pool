/*****************************************************************
* 文件名  : thread_pool.h
* 创建者  ：daibitao
* 创建时间：2107-2-21
* 文件说明：定义线程池相关结构
******************************************************************/
#include <stdio.h>
#include <pthread.h>

/*描述任务*/
typedef struct __pthread_task
{
	void (*process)(void *arg);   /*任务函数指针，参数可调整*/
	void *arg;					  /*函数参数，可以根据需要进行调整*/
	struct __pthread_task *next;  /*指向下一个任务*/
}pthread_task;

/*描述线程池*/
typedef struct __pthread_pool
{
	int max_thread_num;      /*线程池中线程的数目*/
	pthread_t *thread_id;    /*指向创建的线程的首地址*/
	int cur_task_num;        /*当前等待的任务数*/
	pthread_task *queue_head;/*等待的任务队列的头指针*/
	int destroy_pool;        /*记录是否销毁线程池*/

	pthread_mutex_t queue_lock; /*互斥锁*/
	pthread_cond_t queue_cond;  /*条件变量*/
}pthread_pool;

/*1，线程池初始化*/
void pthread_pool_init(pthread_pool *pool, int thread_num);
/*2，往线程池添加任务*/
void pthread_pool_add_task(pthread_pool *pool, void (*process)(void *arg), void *arg);
/*3，注销线程池*/
void pthread_pool_destroy(pthread_pool *pool);
/*4，线程任务处理函数*/
void *thread_routine(void *arg);