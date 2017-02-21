/*****************************************************************
* 文件名  : thread_pool.c
* 创建者  ：daibitao
* 创建时间：2107-2-21
* 文件说明：实现线程池相关函数，应用环境：server端中使用
******************************************************************/
#include "thread_pool.h"

/*****************************************************************
* 函数名  ：pthread_pool_init
* 参数    ：*pool 接收线程池地址
*           thread_num 线程池线程个数
* 返回值  ：无
* 函数功能：初始化线程池
******************************************************************/
void pthread_pool_init(pthread_pool *pool, int thread_num)
{
	int i;

	pool->max_thread_num = thread_num; //初始化线程个数
	
	/*为线程分配空间*/
	pool->thread_id = (pthread_t*)malloc(thread_num*sizeof(pthread_t));
	
	pool->cur_task_num = 0;
	pool->queue_head = NULL;
	pool->destroy_pool = 0; //0不销毁， 1销毁

	/*初始化互斥锁和条件变量*/
	pthread_mutex_init(&(pool->queue_lock), NULL);
	pthread_cond_init(&(pool->queue_cond), NULL);

	/*创建线程*/
	for (i=0; i<thread_num; i++)
		pthread_create(&(pool->thread_id[i]), NULL, thread_routine,(void *)pool);
}

/*****************************************************************
* 函数名  ：thread_routine
* 参数    ：*arg 传进函数的参数，配合pthread_create 使用，
*           参考pthread_pool_init中的调用
* 返回值  ：无
* 函数功能：实现线程功能
******************************************************************/
void *thread_routine(void *arg)
{
	pthread_pool *pool = (pthread_pool *)arg;

	printf("starting thread 0x%x\n", pthread_self());
	while(1)
	{
		pthread_mutex_lock(&(pool->queue_lock));

		while((pool->cur_task_num == 0) && (!pool->destroy_pool))
		{
			printf("thread 0x%x is waiting\n", pthread_self());
			pthread_cond_wait(&(pool->queue_cond), &(pool->queue_lock));
		}

		/*判断线程池是否要销毁*/
		if (pool->destroy_pool)
		{
			pthread_mutex_unlock(&(pool->queue_lock));
			printf("thread 0x%x will exit\n", pthread_self());
			pthread_exit(NULL);
		}

		pool->cur_task_num--;/*任务数减一*/
		pthread_task *tmp_task = pool->queue_head;
		pool->queue_head = tmp_task->next;/*任务向后移*/
		
		/*释放互斥锁*/
		pthread_mutex_unlock(&(pool->queue_lock));

		/*执行任务*/
		(*(tmp_task->process))(tmp_task->arg);
		
		/*释放指针*/
		free(tmp_task);
		tmp_task = NULL;
	}
}

/*****************************************************************
* 函数名  ：pthread_pool_add_task
* 参数    ：*pool 接收线程池地址
*			*process 任务的执行函数，参数类型可变
*           *arg 函数Process参数，类型可变
* 返回值  ：无
* 函数功能：添加任务到线程池
******************************************************************/
void pthread_pool_add_task(pthread_pool *pool, void (*process)(void *arg), void *arg)
{
	pthread_task *tmp_task, *task;

	task = (pthread_task *)malloc(sizeof(pthread_task));
	task->process = process;
	task->arg = arg;
	task->next = NULL;

	/*获取互斥锁*/
	pthread_mutex_lock(&(pool->queue_lock));

	/*添加任务到等待队列中*/
	if (pool->queue_head == NULL)
		pool->queue_head = task;
	else
	{	tmp_task = pool->queue_head;
		while(tmp_task->next != NULL)
			tmp_task = tmp_task->next;
		tmp_task->next = task;
	}
	
	pool->cur_task_num++;

	/*释放互斥锁,并唤醒线程*/
	pthread_mutex_unlock(&(pool->queue_lock));
	pthread_cond_signal(&(pool->queue_cond));
}

/*****************************************************************
* 函数名  ：pthread_pool_destroy
* 参数    ：*pool 接收线程池地址
* 返回值  ：-1 异常返回；0正常返回
* 函数功能：销毁线程池
******************************************************************/
int pthread_pool_destroy(pthread_pool *pool)
{
	/*防止重复调用*/
	if (pool->destroy_pool)
		return -1;
	pool->destroy_pool = 1;

	/*唤醒所有线程*/
	pthread_cond_broadcast(&(pool->queue_cond));

	/*阻塞进程，等待线程退出*/
	int i;
	for (i = 0; i<pool->max_thread_num; i++)
		pthread_join(pool->thread_id[i], NULL);
	free(pool->thread_id);

	/*销毁等待队列*/
	pthread_task *tmp_task = NULL;
	while(pool->queue_head != NULL)
	{
		tmp_task = pool->queue_head;
		pool->queue_head = tmp_task->next;
		free(tmp_task);
	}

	/*销毁互斥锁和条件变量*/
	pthread_mutex_destroy(&(pool->queue_lock));
	pthread_cond_destroy(&(pool->queue_cond));
	
	return 0;
}


/*------------------------------------end of thread pool------------------------------*/