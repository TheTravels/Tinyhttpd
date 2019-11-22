/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : epoll.h
* Author             : Merafour
* Last Modified Date : 11/15/2019
* Description        : epoll.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/

#ifndef EPOLL_SERVER_H_
#define EPOLL_SERVER_H_

#include <stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<sys/types.h>
#include "epoll.h"
#include "../agreement/obd_agree_fops.h"
#include "../agreement/msg_print.h"

#ifdef __cplusplus
extern "C" {
#endif

struct epoll_thread_data{
    int flag;
    int fd;
    char _obd_obj_buf[sizeof(struct obd_agree_obj)];
    struct obd_agree_obj* _obd_obj;
};


extern struct epoll_obj* epoll_listen_init(void* const _epoll_buf);
extern struct epoll_obj* epoll_server_init(void* const _epoll_buf, void* const data);
extern struct epoll_obj* epoll_client_init(void* const _epoll_buf, void* const data);

#ifdef __cplusplus
}
#endif

#endif /* EPOLL_SERVER_H_ */
