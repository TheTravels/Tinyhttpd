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
#include "epoll_server.h"

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
            _this->fops.do_write(_this, fd, buf, strlen(buf));
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
    int i;
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
        for(i=0; i<epoll_obj_list_size; i++)
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
    close(_this->epollfd);
}
static struct epoll_thread_data* get_epoll_data(struct epoll_obj* const _this, const int fd)
{
    int i;
    struct epoll_thread_data* const _data_list = (struct epoll_thread_data*)_this->data;
    struct epoll_thread_data* _data=NULL;
    for(i=0; i<epoll_obj_data_size; i++)
    {
        _data = &_data_list[i];
        if(fd==_data->fd)
        {
            return _data;
        }
    }
    for(i=0; i<epoll_obj_data_size; i++)
    {
        _data = &_data_list[i];
        if(0==_data->flag)
        {
            _data->fd = fd;
            _data->flag = 1;
            _data->_obd_obj = NULL;
            return _data;
        }
    }
    return NULL;
}
static void epoll_close_server(struct epoll_obj* const _this, const int fd)
{
    struct epoll_thread_data* const _thread_data = get_epoll_data(_this, fd);
    memset(_thread_data, 0, sizeof(struct epoll_thread_data));
}
void epoll_get_vin(struct epoll_obj* const _this, struct obd_agree_obj* const _obd_obj, const char sn[], const char VIN[])
{
    (void)_this;
    char vin[32];
    //struct obd_agree_obj* const _obd_obj = conn->_obd_obj;
    if((NULL==sn) || (strlen(sn)<12)) return;
    if((NULL==VIN) || (17==strlen(VIN))) return;
    memset(vin, 0, sizeof(vin));
    // 检查 VIN 码是否存在
    if(0!=_obd_obj->fops->base->vin.search(sn, vin))
    {
        // 添加请求
        _obd_obj->fops->base->vin.req_add(sn);
    }
}
static void handle_events_server(struct epoll_obj* const _this, struct epoll_event* const events, const int num, const int listenfd, char* const buf, const int _max_size)
{
    int i, fd;
    //char _obd_obj_buf[sizeof(struct obd_agree_obj)];
    //struct obd_agree_obj* const _obd_obj = obd_agree_obj_yunjing.fops->constructed(&obd_agree_obj_yunjing, _obd_obj_buf);
    //struct obd_agree_obj* const _obd_obj = obd_agree_obj_shanghai.fops->constructed(&obd_agree_obj_shanghai, _obd_obj_buf);

    char __stream[1024*30] = "\0";
    struct msg_print_obj _print = {
        .fops = &_msg_print_fops,
        .__stream = __stream,
        .__n = sizeof(__stream),
    };
    struct sql_storage_item _items_report[64];      // 数据项
    char _db_report_buf[sizeof(struct data_base_obj)];
    struct data_base_obj* const _db_report = db_obj_report.fops->constructed(&db_obj_report, _db_report_buf, sql_items_format_report, _sql_items_format_report_size, _items_report, _sql_items_format_report_size, "tbl_obd_4g");
    struct obd_agree_ofp_data _ofp_data = {
        ._tbuf = {0},
        ._tlen = 0,
        //._db_report = _db_report,
    };
    struct epoll_thread_data* _thread_data = NULL;
    uint8_t msg_buf[4096];
    //_db_report.fops->init(_db_report);
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
            //struct epoll_thread_data* const _thread_data = (struct epoll_thread_data*)events[i].data.ptr;
            _thread_data = get_epoll_data(_this, fd);
            memset(buf, 0, _max_size);
            nread = _this->fops.do_read(_this, fd, buf, _max_size);
            if(NULL==_thread_data) continue;
            if(nread>0)
            {
                // handle
                struct obd_agree_obj* _obd_obj = _thread_data->_obd_obj;
                memset(&_ofp_data._tbuf, 0, sizeof(_ofp_data._tbuf));
                _print.fops->init(&_print);
                if(NULL!=_thread_data->_obd_obj)
                {
                    //struct obd_agree_obj* _obd_obj = _thread_data->_obd_obj;
                    _print.fops->print(&_print, "协议类型：%s protocol:%d\n", _obd_obj->fops->agree_des, _obd_obj->fops->protocol); fflush(stdout);
                    decode = _obd_obj->fops->decode_server(_obd_obj, (const uint8_t *)buf, nread, msg_buf, sizeof(msg_buf), &_ofp_data, _db_report, &_print);
                }
                else // 判断协议类型
                {
                    // 逐个协议遍历
                    //struct obd_agree_obj* _obd_obj = NULL;
                    //_obd_obj = obd_agree_obj_shanghai.fops->constructed(&obd_agree_obj_shanghai, _thread_data->_obd_obj_buf);
                    _obd_obj = obd_agree_obj_yunjing.fops->constructed(&obd_agree_obj_yunjing, _thread_data->_obd_obj_buf);
                    _print.fops->print(&_print, "逐个协议遍历 : %s protocol:%d\n", _obd_obj->fops->agree_des, _obd_obj->fops->protocol); fflush(stdout);
                    decode = _obd_obj->fops->decode_server(_obd_obj, (const uint8_t *)buf, nread, msg_buf, sizeof(msg_buf), &_ofp_data, _db_report, &_print);
                    if(0==decode) _thread_data->_obd_obj = _obd_obj;
                    if(0!=decode)
                    {
                        //_obd_obj = obd_agree_obj_yunjing.fops->constructed(&obd_agree_obj_yunjing, _thread_data->_obd_obj_buf);
                        _obd_obj = obd_agree_obj_shanghai.fops->constructed(&obd_agree_obj_shanghai, _thread_data->_obd_obj_buf);
                        _print.fops->print(&_print, "逐个协议遍历 : %s protocol:%d\n", _obd_obj->fops->agree_des, _obd_obj->fops->protocol); fflush(stdout);
                        //decode = decode_server(&print, _agree_obd_yj, (const uint8_t *)data, numchars-len, msg_buf, sizeof(msg_buf), device, csend, _print_buf, _print_bsize);
                        decode = _obd_obj->fops->decode_server(_obd_obj, (const uint8_t *)buf, nread, msg_buf, sizeof(msg_buf), &_ofp_data, _db_report, &_print);
                        if(0==decode) _thread_data->_obd_obj = _obd_obj;
                    }
                }
                if(_ofp_data._tlen>10)
                {
                    printf("@%s-%d Send to client: %3d:%s\n", __func__, __LINE__, _ofp_data._tlen, _ofp_data._tbuf);
                    //write(fd, _ofp_data._tbuf, _ofp_data._tlen);
                    _this->fops.do_write(_this, fd, _ofp_data._tbuf, _ofp_data._tlen);
                }
                epoll_get_vin(_this, _obd_obj, _obd_obj->sn, _obd_obj->VIN);
            }
        }
        else if(events[i].events & EPOLLOUT)
        {
            _this->fops.do_write(_this, fd, buf, strlen(buf));
        }
    }
    //_db_report.fops->insert_sql(_db_report);
    //_db_report.fops->close(_db_report);
}

//监听
//static struct epoll_obj* epoll_obj_listen=NULL;
//static char epoll_obj_listen_buf[sizeof(struct epoll_obj)];
//server
//struct epoll_obj* epoll_obj_server=NULL;
//static char epoll_obj_server_buf[sizeof(struct epoll_obj)];

struct epoll_obj* epoll_listen_init(void* const _epoll_buf)
{
    struct epoll_obj* _epoll=NULL;
    _epoll = epoll_obj_base.fops.constructed(&epoll_obj_base, _epoll_buf, do_epoll_listen, handle_events_listen, handle_accept_listen, NULL, NULL);
    return _epoll;
}
struct epoll_obj* epoll_server_init(void* const _epoll_buf, void* const data)
{
    struct epoll_obj* _epoll=NULL;
    _epoll = epoll_obj_base.fops.constructed(&epoll_obj_base, _epoll_buf, do_epoll_server, handle_events_server, NULL, epoll_close_server, data);
    return _epoll;
}

