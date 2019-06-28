#include <stdio.h>
#include <pthread.h>
#include "lock.h"

static pthread_mutex_t mutex;
int pthread_lock_init(void)
{
	return pthread_mutex_init(&mutex, NULL);
}

int pthread_lock(void)
{
	return pthread_mutex_lock(&mutex);
}

int pthread_unlock(void)
{
	return pthread_mutex_unlock(&mutex);
}








