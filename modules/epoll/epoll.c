/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : epoll.c
* Author             : Merafour
* Last Modified Date : 11/15/2019
* Description        : epoll.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
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

#define IPADDRESS "127.0.0.1"
#define PORT 6666
//#define MAXSIZE 1024
//#define LISTENQ 5
//#define FDSIZE 1000
//#define EPOLLEVENTS 100

//函数声明
static int socket_bind(const char* ip, int port);
//void do_epoll(int listenfd);
//void handle_events(int epollfd, struct epoll_event* events, int num, int listenfd, char* buf);
//void handle_accept(int epollfd, int listenfd);
//void do_read(int epollfd, int fd, char* buf);
//void do_write(int epollfd, int fd, char* buf);
//void add_event(int epollfd, int fd, int state);
//void modify_event(int epollfd, int fd, int state);
//void delete_event(int epollfd, int fd, int state);

int epoll_main(int argc, char* argv[])
{
    struct epoll_obj* _epoll_obj=NULL;
    char _epoll_obj_buf[sizeof(struct epoll_obj)];
    _epoll_obj = epoll_obj_base.fops.constructed(&epoll_obj_base, _epoll_obj_buf, NULL, NULL, NULL);
	int listenfd;
	listenfd = socket_bind(IPADDRESS, PORT);   //绑定socket
	listen(listenfd, LISTENQ);       //监听
    _epoll_obj->do_epoll(_epoll_obj, listenfd);
	return 0;
}

static int socket_bind(const char* ip, int port)
{
	int listenfd;
	struct sockaddr_in servaddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd == -1)
	{
		perror("socker error:");
		exit(1);
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &servaddr.sin_addr);
	servaddr.sin_port = htons(port);
	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
	{
		perror("bind error:");
		exit(1);
	}
	return listenfd;
}
//监听
static void epoll_do_epoll(struct epoll_obj* const _this, int listenfd)
{
    //int epollfd;
    //struct epoll_event events[EPOLLEVENTS];
	int ret;
	char buf[MAXSIZE];
	memset(buf, 0, MAXSIZE);
    _this->epollfd = epoll_create(FDSIZE);
    _this->fops.add_event(_this, listenfd, EPOLLIN);
    printf("[%s-%d] \n", __func__, __LINE__);
    while(1)
	{
        ret = epoll_wait(_this->epollfd, _this->events, EPOLLEVENTS, -1);
        _this->handle_events(_this, _this->events, ret, listenfd, buf, sizeof(buf));
	}
    close(_this->epollfd);
}
static void epoll_handle_events(struct epoll_obj* const _this, struct epoll_event* const events, const int num, const int listenfd, char* const buf, const int _max_size)
{
	int i, fd;
	for(i = 0; i<num; i++)
	{
		fd = events[i].data.fd;
		if((fd == listenfd) && (events[i].events & EPOLLIN))
            _this->handle_accept(_this, listenfd);
		else if(events[i].events & EPOLLIN)
            _this->fops.do_read(_this, fd, buf, _max_size);
		else if(events[i].events & EPOLLOUT)
            _this->fops.do_write(_this, fd, buf);
	}
}
static void epoll_handle_accept(struct epoll_obj* const _this, int listenfd)
{
	int clifd;
	struct sockaddr_in cliaddr;
	socklen_t cliaddrlen;
	clifd = accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
	if(clifd == -1)
		perror("accept error:");
	else
	{
        printf("[%s-%d] accept a new client: %s:%d\n", __func__, __LINE__, inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
        _this->fops.add_event(_this, clifd, EPOLLIN);  //  这里应该添加到处理线程
	}
}

static int epoll_do_read(struct epoll_obj* const _this, int const fd, char* const buf, const int _max_size)
{
	int nread;
    nread = read(fd, buf, _max_size);
	if(nread == -1)
	{
		perror("read error:");
		close(fd);
        _this->fops.delete_event(_this, fd, EPOLLIN);
	}
	else if(nread == 0)
	{
		fprintf(stderr, "client close.\n");
		close(fd);
        _this->fops.delete_event(_this, fd, EPOLLIN);
	}
	else
	{
		printf("read message is : %s", buf);
        _this->fops.modify_event(_this, fd, EPOLLOUT);
	}
    return nread;
}

static void epoll_do_write(struct epoll_obj* const _this, int fd, char* buf)
{
	int nwrite;
	nwrite = write(fd, buf, strlen(buf));
	if(nwrite == -1)
	{
		perror("write error:");
		close(fd);
        _this->fops.delete_event(_this, fd, EPOLLIN);
	}
	else
        _this->fops.modify_event(_this, fd, EPOLLIN);
	memset(buf, 0, MAXSIZE);
}

static void epoll_add_event(struct epoll_obj* const _this, int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
    epoll_ctl(_this->epollfd, EPOLL_CTL_ADD, fd, &ev);
    _this->fd_count++;
    printf("[%s-%d] \n", __func__, __LINE__);
}

static void epoll_delete_event(struct epoll_obj* const _this, int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
    epoll_ctl(_this->epollfd, EPOLL_CTL_DEL, fd, &ev);
    _this->fd_count--;
    printf("[%s-%d] \n", __func__, __LINE__);
}

static void epoll_modify_event(struct epoll_obj* const _this, int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
    epoll_ctl(_this->epollfd, EPOLL_CTL_MOD, fd, &ev);
    printf("[%s-%d] \n", __func__, __LINE__);
}

static struct epoll_obj* epoll_obj_list[epoll_obj_list_size];
static int add_epoll_obj(struct epoll_obj* const _this)
{
    int index;
    struct epoll_obj* _obj=NULL;
    printf("[%s-%d] \n", __func__, __LINE__);
    for(index=0; index<epoll_obj_list_size; index++)
    {
        _obj=epoll_obj_list[index];
        if(NULL == _obj)
        {
            printf("[%s-%d] \n", __func__, __LINE__);
            epoll_obj_list[index] = _this;
            return 0;
        }
    }
    return -1;
}
static int del_epoll_obj(struct epoll_obj* const _this)
{
    int index;
    struct epoll_obj* _obj=NULL;
    printf("[%s-%d] \n", __func__, __LINE__);
    for(index=0; index<epoll_obj_list_size; index++)
    {
        _obj=epoll_obj_list[index];
        if(_this == _obj)
        {
            printf("[%s-%d] \n", __func__, __LINE__);
            epoll_obj_list[index] = NULL;
            return 0;
        }
    }
    return -1;
}
static struct epoll_obj* get_epoll_obj(struct epoll_obj* const _this, const int _index)
{
    if(_index<epoll_obj_list_size) return epoll_obj_list[_index];
    return NULL;
}

// 构造函数
//static struct config_load_obj* __this_constructed(struct config_load_obj* const _load_obj, void* const _obj_buf, const config_load_func_t _load_func, const char _cfg_path[], char* const _stream, const size_t _n, void* const _data)
static struct epoll_obj* __this_constructed(struct epoll_obj* const _this, void* const _obj_buf, const epoll_do_epoll_func_t _do_epoll, const epoll_handle_events_func_t _events, const epoll_handle_accept_func_t _accept)
{
    struct epoll_obj _fops = {
        .fops = _this->fops,
        .do_epoll = _do_epoll,
        .handle_events = _events,
        .handle_accept = _accept,
        .epollfd = 0,
        .fd_count = 0,
    };
    struct config_load_obj* const _obj = (struct config_load_obj*)_obj_buf;
    //printf("[%s-%d] _n:%d\n", __func__, __LINE__, _n);  fflush(stdout);
    memset(_fops.events, 0, sizeof(_fops.events));
    memcpy(_obj_buf, &_fops, sizeof(_fops));
    return _obj;
}
static struct epoll_obj* this_constructed(struct epoll_obj* const _this, void* const _obj_buf, const epoll_do_epoll_func_t _do_epoll, const epoll_handle_events_func_t _events, const epoll_handle_accept_func_t _accept)
{
    epoll_do_epoll_func_t do_epoll = _do_epoll;
    if(NULL == do_epoll) do_epoll = _this->do_epoll;
    epoll_handle_events_func_t events = _events;
    if(NULL == events) events = _this->handle_events;
    epoll_handle_accept_func_t accept = _accept;
    if(NULL == accept) accept = _this->handle_accept;
    return __this_constructed(_this, _obj_buf, do_epoll, events, accept);
}
//监听
struct epoll_obj epoll_obj_base = {
    .fops = {
        .constructed = &this_constructed,
        .do_read = epoll_do_read,
        .do_write = epoll_do_write,
        .add_event = epoll_add_event,
        .modify_event = epoll_modify_event,
        .delete_event = epoll_delete_event,
        .add_epoll_obj = add_epoll_obj,
        .del_epoll_obj = del_epoll_obj,
        .get_epoll_obj = get_epoll_obj,
    },
    .do_epoll = epoll_do_epoll,
    .handle_events = epoll_handle_events,
    .handle_accept = epoll_handle_accept,
    .epollfd = 0,
    .fd_count = 0,
    .events = {0},
};
