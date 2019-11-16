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

#ifndef EPOLL_H_
#define EPOLL_H_

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

#ifdef __cplusplus
extern "C" {
#endif

#define MAXSIZE 4096
#define LISTENQ 5
//#define FDSIZE 1000
//#define EPOLLEVENTS 100
#define FDSIZE 10000
#define EPOLLEVENTS 4096

struct epoll_obj;
typedef void (*epoll_do_epoll_func_t)(struct epoll_obj* const _this, int listenfd);
typedef void (*epoll_handle_events_func_t)(struct epoll_obj* const _this, struct epoll_event* const events, const int num, const int listenfd, char* const buf, const int _max_size);
typedef void (*epoll_handle_accept_func_t)(struct epoll_obj* const _this, int listenfd);

// epoll 个数
#define epoll_obj_list_size   128
// epoll 中数据即连接个数，每个连接对应一个数据
#define epoll_obj_data_size   1024

struct epoll_fops{
    // 构造函数
    struct epoll_obj* (*const constructed)(struct epoll_obj* const _this, void* const _obj_buf, const epoll_do_epoll_func_t _do_epoll, const epoll_handle_events_func_t _events, const epoll_handle_accept_func_t _accept, void* const data);
    int (*const do_read)(struct epoll_obj* const _this, int const fd, char* const buf, const int _max_size);
    void (*const do_write)(struct epoll_obj* const _this, int fd, char* buf, const int _size);
    void (*const add_event)(struct epoll_obj* const _this, int fd, int state);
    void (*const modify_event)(struct epoll_obj* const _this, int fd, int state);
    void (*const delete_event)(struct epoll_obj* const _this, int fd, int state);
    //  对 epoll 对象进行管理
    int (*const add_epoll_obj)(struct epoll_obj* const _this);
    int (*const del_epoll_obj)(struct epoll_obj* const _this);
    struct epoll_obj* (*const get_epoll_obj)(struct epoll_obj* const _this, const int _index);
};

struct epoll_obj{
    const struct epoll_fops fops;
    void (*const do_epoll)(struct epoll_obj* const _this, int listenfd);
    void (*const handle_events)(struct epoll_obj* const _this, struct epoll_event* const events, const int num, const int listenfd, char* const buf, const int _max_size);
    void (*const handle_accept)(struct epoll_obj* const _this, int listenfd);
    int epollfd;       // 文件描述符
    int fd_count;      // 文件描述符计数
    struct epoll_event events[EPOLLEVENTS];
    void* const data;
};

extern struct epoll_obj epoll_obj_base;
//extern struct epoll_obj* epoll_listen_init(void);
//extern struct epoll_obj* epoll_server_init(void* const _epoll_buf, void* const data);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_LOAD_H_ */
