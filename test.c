#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "thread_pool.h"

void test_process(void *arg)
{
	printf("thread_id is 0x%x, working on task %d\n", pthread_self(), *(int *)arg);
	sleep(1);
}

int main(int argc, char **argv)
{	
	int i;
	pthread_pool test_pool;

	pthread_pool_init(&test_pool, 3);
	sleep(5);

	int *work_num = (int *)malloc(sizeof(int)*10);


	for (i=0; i<10; i++)
	{
		work_num[i] = i;
		pthread_pool_add_task(&test_pool, test_process, (void *)(&work_num[i]));
	}
	sleep(5);
	pthread_pool_destroy(&test_pool);

	free(work_num);
	return 0;
}