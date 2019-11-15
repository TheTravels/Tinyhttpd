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
#include "epoll.h"

#define MAXSIZE 1024
//#define LISTENQ 5



//监听
static void do_epoll_listen(struct epoll_obj* const _this, int listenfd)
{
	int ret;
	char buf[MAXSIZE];
	memset(buf, 0, MAXSIZE);
    _this->epollfd = epoll_create(FDSIZE);
    _this->fops.add_event(_this, listenfd, EPOLLIN);
    while(1)
	{
        ret = epoll_wait(_this->epollfd, _this->events, EPOLLEVENTS, -1);
        _this->handle_events(_this, _this->events, ret, listenfd, buf);
	}
    close(_this->epollfd);
}
static void handle_events_listen(struct epoll_obj* const _this, struct epoll_event* events, int num, int listenfd, char* buf)
{
	int i, fd;
	for(i = 0; i<num; i++)
	{
		fd = events[i].data.fd;
		if((fd == listenfd) && (events[i].events & EPOLLIN))
            _this->handle_accept(_this, listenfd);
		else if(events[i].events & EPOLLIN)
            _this->fops.do_read(_this, fd, buf);
		else if(events[i].events & EPOLLOUT)
            _this->fops.do_write(_this, fd, buf);
	}
}
static void handle_accept_listen(struct epoll_obj* const _this, int listenfd)
{
	int clifd;
	struct sockaddr_in cliaddr;
	socklen_t cliaddrlen;
    struct epoll_obj* _obj=NULL;
    struct epoll_obj* _obj_min=NULL;
    int fd_count;      // 文件描述符计数
	clifd = accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
	if(clifd == -1)
		perror("accept error:");
	else
	{
		printf("accept a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
        //_this->fops.add_event(_this, clifd, EPOLLIN);  //  这里应该添加到处理线程
        // 遍历,查找连接数最少的对象
        fd_count = 10000;
        _obj_min=NULL;
        for(int i=0; i<epoll_obj_list_size; i++)
        {
            _obj=_this->fops.get_epoll_obj(_this, i);
            if(NULL==_obj) continue;
            if(fd_count>_obj->fd_count)
            {
                fd_count=_obj->fd_count;
                _obj_min=_obj;
            }
        }
        if(NULL!=_obj_min) _obj_min->fops.add_event(_obj_min, clifd, EPOLLIN);  // 添加到处理线程
	}
}
//Server
static void do_epoll_server(struct epoll_obj* const _this, int listenfd)
{
    int ret;
    char buf[MAXSIZE];
    (void)listenfd;
    memset(buf, 0, MAXSIZE);
    _this->epollfd = epoll_create(FDSIZE);
    //_this->fops.add_event(_this, listenfd, EPOLLIN);
    _this->fops.add_epoll_obj(_this);
    while(1)
    {
        while(0>=_this->fd_count) sleep(1);  // 没有连接需要处理，休眠
        ret = epoll_wait(_this->epollfd, _this->events, EPOLLEVENTS, -1);
        _this->handle_events(_this, _this->events, ret, listenfd, buf);
    }
    //close(_this->epollfd);
}
static void handle_events_server(struct epoll_obj* const _this, struct epoll_event* events, int num, int listenfd, char* buf)
{
    int i, fd;
    for(i = 0; i<num; i++)
    {
        fd = events[i].data.fd;
        if((fd == listenfd) && (events[i].events & EPOLLIN))
            _this->handle_accept(_this, listenfd);
        else if(events[i].events & EPOLLIN)
            _this->fops.do_read(_this, fd, buf);
        else if(events[i].events & EPOLLOUT)
            _this->fops.do_write(_this, fd, buf);
    }
}

//监听
struct epoll_obj* epoll_obj_listen=NULL;
static char epoll_obj_listen_buf[sizeof(struct epoll_obj)];
//server
struct epoll_obj* epoll_obj_server=NULL;
static char epoll_obj_server_buf[sizeof(struct epoll_obj)];

int epoll_server_init(void)
{
    epoll_obj_listen = epoll_obj_base.fops.constructed(&epoll_obj_base, epoll_obj_listen_buf, do_epoll_listen, handle_events_listen, handle_accept_listen);
    epoll_obj_server = epoll_obj_base.fops.constructed(&epoll_obj_base, epoll_obj_server_buf, do_epoll_server, handle_events_server, NULL);
    return 0;
}
