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

int connect_server(const char host, const int port)
{
    int sockfd;
    struct sockaddr_in servaddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, host, &servaddr.sin_addr);
    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    //handle_connection(sockfd);
    //close(sockfd);
    return sockfd;
}

//Client
static void do_epoll_client(struct epoll_obj* const _this, int listenfd)
{
    int ret;
    int sockfd;
    char buf[MAXSIZE];
    memset(buf, 0, MAXSIZE);
    _this->epollfd = epoll_create(FDSIZE);
    printf("[%s-%d] \n", __func__, __LINE__);
    sockfd = connect_server("183.237.191.186", 6100);
    _this->fops.add_event(_this, sockfd, EPOLLIN);
    printf("[%s-%d] \n", __func__, __LINE__);
    _this->fops.modify_event(_this, listenfd, EPOLLOUT);
    while(1)
    {
        ret = epoll_wait(_this->epollfd, _this->events, EPOLLEVENTS, -1);
        printf("[%s-%d] \n", __func__, __LINE__);
        _this->handle_events(_this, _this->events, ret, listenfd, buf, sizeof(buf));
    }
    close(_this->epollfd);
}
/*static void handle_events_client(struct epoll_obj* const _this, struct epoll_event* const events, const int num, const int listenfd, char* const buf, const int _max_size)
{
    int i, fd;
    for(i = 0; i<num; i++)
    {
        fd = events[i].data.fd;
        if(events[i].events & EPOLLIN)
        {
            _this->fops.do_read(_this, fd, buf, _max_size);
        }
        else if(events[i].events & EPOLLOUT)
        {
            _this->fops.do_write(_this, fd, buf, strlen(buf));
        }
    }
}*/

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
static void handle_events_client(struct epoll_obj* const _this, struct epoll_event* const events, const int num, const int listenfd, char* const buf, const int _max_size)
{
    int i, fd;
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
                memset(&_ofp_data._tbuf, 0, sizeof(_ofp_data._tbuf));
                _print.fops->init(&_print);
                if(NULL!=_thread_data->_obd_obj)
                {
                    struct obd_agree_obj* _obd_obj = _thread_data->_obd_obj;
                    _print.fops->print(&_print, "协议类型：%s protocol:%d\n", _obd_obj->fops->agree_des, _obd_obj->fops->protocol); fflush(stdout);
                    decode = _obd_obj->fops->decode_server(_obd_obj, (const uint8_t *)buf, nread, msg_buf, sizeof(msg_buf), &_ofp_data, _db_report, &_print);
                }
                if(_ofp_data._tlen>10)
                {
                    printf("@%s-%d Send to client: %3d:%s\n", __func__, __LINE__, _ofp_data._tlen, _ofp_data._tbuf);
                    //write(fd, _ofp_data._tbuf, _ofp_data._tlen);
                    _this->fops.do_write(_this, fd, _ofp_data._tbuf, _ofp_data._tlen);
                }
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

struct epoll_obj* epoll_client_init(void* const _epoll_buf, void* const data)
{
    struct epoll_obj* _epoll=NULL;
    //_epoll = epoll_obj_base.fops.constructed(&epoll_obj_base, _epoll_buf, do_epoll_listen, handle_events_listen, handle_accept_listen, NULL);
    _epoll = epoll_obj_base.fops.constructed(&epoll_obj_base, _epoll_buf, do_epoll_client, handle_events_client, NULL, NULL, data);
    return _epoll;
}

