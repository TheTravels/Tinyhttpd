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
#include "../agreement/obd_agree_fops.h"
#include "../agreement/msg_print.h"

//#define MAXSIZE 4096
//#define LISTENQ 5

//监听
static void do_epoll_listen(struct epoll_obj* const _this, int listenfd)
{
	int ret;
	char buf[MAXSIZE];
	memset(buf, 0, MAXSIZE);
    _this->epollfd = epoll_create(FDSIZE);
    _this->fops.add_event(_this, listenfd, EPOLLIN);
    printf("[%s-%d] \n", __func__, __LINE__);
    while(1)
	{
        ret = epoll_wait(_this->epollfd, _this->events, EPOLLEVENTS, -1);
        printf("[%s-%d] \n", __func__, __LINE__);
        _this->handle_events(_this, _this->events, ret, listenfd, buf, sizeof(buf));
	}
    close(_this->epollfd);
}
static void handle_events_listen(struct epoll_obj* const _this, struct epoll_event* const events, const int num, const int listenfd, char* const buf, const int _max_size)
{
	int i, fd;
	for(i = 0; i<num; i++)
	{
		fd = events[i].data.fd;
		if((fd == listenfd) && (events[i].events & EPOLLIN))
            _this->handle_accept(_this, listenfd);
		else if(events[i].events & EPOLLIN)
        {
            _this->fops.do_read(_this, fd, buf, _max_size);
        }
		else if(events[i].events & EPOLLOUT)
        {
            _this->fops.do_write(_this, fd, buf);
        }
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
        printf("[%s-%d] accept a new client: %s:%d\n", __func__, __LINE__, inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
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
        if(NULL!=_obj_min)
        {
            printf("[%s-%d] fd_count:%d\n", __func__, __LINE__, _obj_min->fd_count);
            _obj_min->fops.add_event(_obj_min, clifd, EPOLLIN);  // 添加到处理线程
        }
        //else if(_this->fd_count<20) _this->fops.add_event(_this, clifd, EPOLLIN);  // 添加到处理线程
        else
        {
            // delete
            printf("[%s-%d] \n", __func__, __LINE__);
            close(clifd);
        }
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
    //printf("[%s-%d] \n", __func__, __LINE__);
    _this->fops.add_epoll_obj(_this);
    //printf("[%s-%d] \n", __func__, __LINE__);
    while(1)
    {
        while(0>=_this->fd_count) sleep(1);  // 没有连接需要处理，休眠
        ret = epoll_wait(_this->epollfd, _this->events, EPOLLEVENTS, -1);
        _this->handle_events(_this, _this->events, ret, listenfd, buf, sizeof(buf));
    }
    //close(_this->epollfd);
}
static void handle_events_server(struct epoll_obj* const _this, struct epoll_event* const events, const int num, const int listenfd, char* const buf, const int _max_size)
{
    int i, fd;
    char _obd_obj_buf[sizeof(struct obd_agree_obj)];
    //struct obd_agree_obj* const _obd_obj = obd_agree_obj_yunjing.fops->constructed(&obd_agree_obj_yunjing, _obd_obj_buf);
    struct obd_agree_obj* const _obd_obj = obd_agree_obj_shanghai.fops->constructed(&obd_agree_obj_shanghai, _obd_obj_buf);
    char __stream[1024*30] = "\0";
    struct msg_print_obj _print = {
        .fops = &_msg_print_fops,
        .__stream = __stream,
        .__n = sizeof(__stream),
    };
    struct obd_agree_ofp_data _ofp_data;
    uint8_t msg_buf[4096];
    for(i = 0; i<num; i++)
    {
        fd = events[i].data.fd;
        if((fd == listenfd) && (events[i].events & EPOLLIN))
        {
            _this->handle_accept(_this, listenfd);
        }
        else if(events[i].events & EPOLLIN)
        {
            int nread;
            int decode; // 解码数据
            memset(buf, 0, _max_size);
            nread = _this->fops.do_read(_this, fd, buf, _max_size);
            if(nread>0)
            {
                // handle
                memset(&_ofp_data, 0, sizeof(_ofp_data));
                _print.fops->init(&_print);
                _print.fops->print(&_print, "协议类型：上海OBD协议 device->protocol:%d\n", _obd_obj->fops->protocol); fflush(stdout);
                decode = _obd_obj->fops->decode_server(_obd_obj, (const uint8_t *)buf, nread, msg_buf, sizeof(msg_buf), &_ofp_data, &_print);
                //switch(device->protocol) // 判断协议类型
                /*// 逐个协议遍历
                msg_print(_print_buf, _print_bsize, "逐个协议遍历 device->protocol:%d\n", device->protocol); fflush(stdout);
                decode = _obd_obj->fops->decode_server(_obd_obj, (const uint8_t *)data, numchars-len, msg_buf, sizeof(msg_buf), &_ofp_data, &_print);
                if(0==decode) device->protocol = PRO_TYPE_SHH;
                if(0!=decode)
                {
                    msg_print(_print_buf, _print_bsize, "逐个协议遍历 device->protocol:%d\n", device->protocol); fflush(stdout);
                    //decode = decode_server(&print, _agree_obd_yj, (const uint8_t *)data, numchars-len, msg_buf, sizeof(msg_buf), device, csend, _print_buf, _print_bsize);
                    decode = _obd_obj->fops->decode_server(_obd_obj, (const uint8_t *)data, numchars-len, msg_buf, sizeof(msg_buf), &_ofp_data, &_print);
                    if(0==decode) device->protocol = PRO_TYPE_YJ;
                }*/
                if(_ofp_data._tlen>10)
                {
                    printf("@%s-%d Send to client: %3d:%s\n", __func__, __LINE__, _ofp_data._tlen, _ofp_data._tbuf);
                    write(fd, _ofp_data._tbuf, _ofp_data._tlen);
                }
            }
        }
        else if(events[i].events & EPOLLOUT)
        {
            _this->fops.do_write(_this, fd, buf);
        }
    }
}

//监听
static struct epoll_obj* epoll_obj_listen=NULL;
static char epoll_obj_listen_buf[sizeof(struct epoll_obj)];
//server
//struct epoll_obj* epoll_obj_server=NULL;
//static char epoll_obj_server_buf[sizeof(struct epoll_obj)];

struct epoll_obj* epoll_listen_init(void)
{
    epoll_obj_listen = epoll_obj_base.fops.constructed(&epoll_obj_base, epoll_obj_listen_buf, do_epoll_listen, handle_events_listen, handle_accept_listen);
    return epoll_obj_listen;
}
struct epoll_obj* epoll_server_init(void* const _epoll_buf)
{
    struct epoll_obj* _epoll=NULL;
    _epoll = epoll_obj_base.fops.constructed(&epoll_obj_base, _epoll_buf, do_epoll_server, handle_events_server, NULL);
    return _epoll;
}

