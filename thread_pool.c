/*****************************************************************
* �ļ���  : thread_pool.c
* ������  ��daibitao
* ����ʱ�䣺2107-2-21
* �ļ�˵����ʵ���̳߳���غ�����Ӧ�û�����server����ʹ��
******************************************************************/
#include "thread_pool.h"

/*****************************************************************
* ������  ��pthread_pool_init
* ����    ��*pool �����̳߳ص�ַ
*           thread_num �̳߳��̸߳���
* ����ֵ  ����
* �������ܣ���ʼ���̳߳�
******************************************************************/
void pthread_pool_init(pthread_pool *pool, int thread_num)
{
	int i;

	pool->max_thread_num = thread_num; //��ʼ���̸߳���
	
	/*Ϊ�̷߳���ռ�*/
	pool->thread_id = (pthread_t*)malloc(thread_num*sizeof(pthread_t));
	
	pool->cur_task_num = 0;
	pool->queue_head = NULL;
	pool->destroy_pool = 0; //0�����٣� 1����

	/*��ʼ������������������*/
	pthread_mutex_init(&(pool->queue_lock), NULL);
	pthread_cond_init(&(pool->queue_cond), NULL);

	/*�����߳�*/
	for (i=0; i<thread_num; i++)
		pthread_create(&(pool->thread_id[i]), NULL, thread_routine,(void *)pool);
}

/*****************************************************************
* ������  ��thread_routine
* ����    ��*arg ���������Ĳ��������pthread_create ʹ�ã�
*           �ο�pthread_pool_init�еĵ���
* ����ֵ  ����
* �������ܣ�ʵ���̹߳���
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

		/*�ж��̳߳��Ƿ�Ҫ����*/
		if (pool->destroy_pool)
		{
			pthread_mutex_unlock(&(pool->queue_lock));
			printf("thread 0x%x will exit\n", pthread_self());
			pthread_exit(NULL);
		}

		pool->cur_task_num--;/*��������һ*/
		pthread_task *tmp_task = pool->queue_head;
		pool->queue_head = tmp_task->next;/*���������*/
		
		/*�ͷŻ�����*/
		pthread_mutex_unlock(&(pool->queue_lock));

		/*ִ������*/
		(*(tmp_task->process))(tmp_task->arg);
		
		/*�ͷ�ָ��*/
		free(tmp_task);
		tmp_task = NULL;
	}
}

/*****************************************************************
* ������  ��pthread_pool_add_task
* ����    ��*pool �����̳߳ص�ַ
*			*process �����ִ�к������������Ϳɱ�
*           *arg ����Process���������Ϳɱ�
* ����ֵ  ����
* �������ܣ���������̳߳�
******************************************************************/
void pthread_pool_add_task(pthread_pool *pool, void (*process)(void *arg), void *arg)
{
	pthread_task *tmp_task, *task;

	task = (pthread_task *)malloc(sizeof(pthread_task));
	task->process = process;
	task->arg = arg;
	task->next = NULL;

	/*��ȡ������*/
	pthread_mutex_lock(&(pool->queue_lock));

	/*������񵽵ȴ�������*/
	if (pool->queue_head == NULL)
		pool->queue_head = task;
	else
	{	tmp_task = pool->queue_head;
		while(tmp_task->next != NULL)
			tmp_task = tmp_task->next;
		tmp_task->next = task;
	}
	
	pool->cur_task_num++;

	/*�ͷŻ�����,�������߳�*/
	pthread_mutex_unlock(&(pool->queue_lock));
	pthread_cond_signal(&(pool->queue_cond));
}

/*****************************************************************
* ������  ��pthread_pool_destroy
* ����    ��*pool �����̳߳ص�ַ
* ����ֵ  ��-1 �쳣���أ�0��������
* �������ܣ������̳߳�
******************************************************************/
int pthread_pool_destroy(pthread_pool *pool)
{
	/*��ֹ�ظ�����*/
	if (pool->destroy_pool)
		return -1;
	pool->destroy_pool = 1;

	/*���������߳�*/
	pthread_cond_broadcast(&(pool->queue_cond));

	/*�������̣��ȴ��߳��˳�*/
	int i;
	for (i = 0; i<pool->max_thread_num; i++)
		pthread_join(pool->thread_id[i], NULL);
	free(pool->thread_id);

	/*���ٵȴ�����*/
	pthread_task *tmp_task = NULL;
	while(pool->queue_head != NULL)
	{
		tmp_task = pool->queue_head;
		pool->queue_head = tmp_task->next;
		free(tmp_task);
	}

	/*���ٻ���������������*/
	pthread_mutex_destroy(&(pool->queue_lock));
	pthread_cond_destroy(&(pool->queue_cond));
	
	return 0;
}


/*------------------------------------end of thread pool------------------------------*/