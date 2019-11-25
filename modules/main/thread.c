#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <pthread.h>  
#include <assert.h>  
#include "../epoll/epoll.h"
#include "../epoll/epoll_server.h"
#include "../config/config_data.h"

static void* epoll_pthread(void *arg)
{  
    struct epoll_obj* _epoll_server=NULL;
    char _epoll_server_buf[sizeof(struct epoll_obj)];
    char _epoll_data[sizeof(struct epoll_thread_data)*(epoll_obj_data_size+10)];
    long _pthread_id = (long)arg;
    //struct local_config_data* _cfg_data = (struct local_config_data*)_local_config_data->data;
    printf("[%s-%d] _pthread_id:%ld\n", __func__, __LINE__, _pthread_id);
    memset(_epoll_server_buf, 0, sizeof(_epoll_server_buf));
    memset(_epoll_data, 0, sizeof(_epoll_data));
    //if((1==_cfg_data->_vin_cfg._turn_on) && (0==_pthread_id))
    /*if(0==_pthread_id)
    {
        //printf("[%s-%d] _pthread_id:%ld _epoll_data:%p\n", __func__, __LINE__, _pthread_id, _epoll_data);
        _epoll_server=epoll_client_init(_epoll_server_buf, _epoll_data);
        while(1)
        {
            _epoll_server->do_epoll(_epoll_server, 0);
        }
    }*/
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














