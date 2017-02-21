/*****************************************************************
* �ļ���  : thread_pool.h
* ������  ��daibitao
* ����ʱ�䣺2107-2-21
* �ļ�˵���������̳߳���ؽṹ
******************************************************************/
#include <stdio.h>
#include <pthread.h>

/*��������*/
typedef struct __pthread_task
{
	void (*process)(void *arg);   /*������ָ�룬�����ɵ���*/
	void *arg;					  /*�������������Ը�����Ҫ���е���*/
	struct __pthread_task *next;  /*ָ����һ������*/
}pthread_task;

/*�����̳߳�*/
typedef struct __pthread_pool
{
	int max_thread_num;      /*�̳߳����̵߳���Ŀ*/
	pthread_t *thread_id;    /*ָ�򴴽����̵߳��׵�ַ*/
	int cur_task_num;        /*��ǰ�ȴ���������*/
	pthread_task *queue_head;/*�ȴ���������е�ͷָ��*/
	int destroy_pool;        /*��¼�Ƿ������̳߳�*/

	pthread_mutex_t queue_lock; /*������*/
	pthread_cond_t queue_cond;  /*��������*/
}pthread_pool;

/*1���̳߳س�ʼ��*/
void pthread_pool_init(pthread_pool *pool, int thread_num);
/*2�����̳߳��������*/
void pthread_pool_add_task(pthread_pool *pool, void (*process)(void *arg), void *arg);
/*3��ע���̳߳�*/
void pthread_pool_destroy(pthread_pool *pool);
/*4���߳���������*/
void *thread_routine(void *arg);