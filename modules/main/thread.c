#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <pthread.h>  
#include <assert.h>  
#include "../epoll/epoll.h"
#include "../epoll/epoll_server.h"

static void* epoll_pthread(void *arg)
{  
    struct epoll_obj* _epoll_server=NULL;
    char _epoll_server_buf[sizeof(struct epoll_obj)];
    char _epoll_data[sizeof(struct epoll_thread_data)*epoll_obj_data_size];
    printf("[%s-%d] _pthread_id:%ld\n", __func__, __LINE__, (long)arg);
    _epoll_server=epoll_server_init(_epoll_server_buf, _epoll_data);
    //printf("[%s-%d] \n", __func__, __LINE__);
    while(1)
    {
        //printf("[%s-%d] \n", __func__, __LINE__);
        _epoll_server->do_epoll(_epoll_server, 0);
    }
    return NULL;
}  

static pthread_t _pthread_id[1024];

void epoll_pthread_init(const int _thread_max)
{
    long i;
    void *arg;
    int _pthread_id_size = sizeof(_pthread_id)/sizeof(_pthread_id[0]);
    int _max = _thread_max;
    if(_max>_pthread_id_size) _max = _pthread_id_size;
    if(_max<2) _max = 2;
    for(i=0; i<_max; i++)
    {
        arg = (void *)i;
        pthread_create (&_pthread_id[i], NULL, epoll_pthread, arg);
    }
}














