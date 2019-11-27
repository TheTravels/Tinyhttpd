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
//设置一个1s延时信号，再注册一个
#include <stdio.h>
#include <signal.h>
#include "../config/config_data.h"
#include "../agreement/msg_print.h"

enum epoll_server_type{
    SERVER_TYPE_VIEW = 0x02,
    SERVER_TYPE_OBD_GEN = 0x03,
    SERVER_TYPE_OBD_YJ = 0x04,
};
static struct epoll_obj* _epoll_client=NULL;
static char __stream[1024*30] = "\0";
static struct msg_print_obj _print = {&_msg_print_fops, "thread_vin_", __stream, sizeof(__stream), };
static time_t epoll_client_systime = 0;
static const int epoll_client_timeout = 3;

static int request_vin(struct epoll_obj* const _this, struct epoll_thread_data* const _data)
{
    time_t tnow;
    time_t timep;
    struct tm *_tmp=NULL;
    _print.fops->init(&_print);
    time(&tnow);
    timep = tnow;
    //printf("@%s-%d\n", __func__, __LINE__);
    if(NULL!=_this)
    {
        struct obd_agree_obj* const _obd_obj = _data->_obd_obj;
        char wsn[32];
        char cache[512];
        const uint32_t cache_size = sizeof(cache);
        char _vin[512];
        //const uint32_t _vsize = sizeof(_vin);
        char _msg_buf[512];
        const uint32_t _msize = sizeof(_msg_buf);
        struct yunjing_userdef _udef;
        int len = 0;
        memset(wsn, 0, sizeof(wsn));
        //printf("@%s-%d\n", __func__, __LINE__);
        //this->get_vin(pPeerConn, conn->_obd_obj->sn, conn->_obd_obj->VIN);
        if(0==_obd_obj->fops->base->vin.req_get(wsn))
        {
            _tmp=localtime(&timep);
            _print.fops->print(&_print, "[%s-%d] request VIN SN[%d-%d-%d %02d:%02d:%02d]:[%s]\n", __func__, __LINE__, _tmp->tm_year+1900, _tmp->tm_mon+1, _tmp->tm_mday, _tmp->tm_hour, _tmp->tm_min, _tmp->tm_sec, wsn);
            //printf("@%s-%d wsn:%s secket:%s\n", __func__, __LINE__, wsn, _conn->localAddress().toIpPort().c_str());
            _print.fops->print(&_print, "@%s-%d wsn:%s secket:%d\n", __func__, __LINE__, wsn, _data->fd);
            //printf("@%s-%d wsn:%s\n", __func__, __LINE__, wsn);
            memset(&_udef, 0, sizeof(struct yunjing_userdef));
            memset(cache, 0, sizeof(cache));
            memset(_vin, 0, sizeof(_vin));
            memset(_msg_buf, 0, sizeof(_msg_buf));
            _udef.type_msg = USERDEF_YJ_QUERY_VIN; // 请求 VIN码
            memcpy(_udef.msg, wsn, 18);
            len = _obd_obj->fops->userdef_encode(_obd_obj, &_udef, cache, cache_size);
            //printf("@%s-%d\n", __func__, __LINE__);
            len = _obd_obj->fops->base->pack.encode(_obd_obj, cache, (uint16_t)len, (uint8_t*)_msg_buf, _msize);
            memset(_obd_obj->sn, 0, sizeof(_obd_obj->sn));
            memcpy(_obd_obj->sn, wsn, strlen(wsn));
            //_conn->send((char*)_msg_buf, len);
            //printf("@%s-%d\n", __func__, __LINE__);
            _this->fops.modify_event(_this, _data->fd, EPOLLOUT);
            //printf("@%s-%d fd:%d\n", __func__, __LINE__, _data->fd);
            _this->fops.do_write(_this, _data->fd, _msg_buf, len);
            //printf("@%s-%d\n", __func__, __LINE__);
            //_print.fops->fflush(&_print);
            return 0;
        }
    }
    return -1;
}
static void timer_run_every(struct epoll_obj* const _this)
{
    int i;
    struct epoll_thread_data* const _data_list = (struct epoll_thread_data*)_this->data;
    struct epoll_thread_data* _data=NULL;
    //int send=-1;
    //printf("[%s-%d] epoll_client_systime:%d\n", __func__, __LINE__, epoll_client_systime);
    for(i=0; i<epoll_obj_data_size; i++)
    {
        _data = &_data_list[i];
        if(0==_data->flag)
        {
            continue;
        }
        //_data->_timeout++;
        if((NULL!=_this) && (_data->_timeout<epoll_client_systime)) // 3s timeout
        {
#if 0
            send=request_vin(_this, _data);
            if(0==send) _data->_timeout = 0;
#else
            //printf("[%s-%d] fd:%d epoll_client_systime:%d _timeout:%d\n", __func__, __LINE__, _data->fd, epoll_client_systime, _data->_timeout);
            request_vin(_this, _data);
            //_this->fops.modify_event(_this, _data->fd, EPOLLOUT);
            _data->_timeout = epoll_client_systime + epoll_client_timeout;
#endif
        }
    }
}

static void epoll_timer(int sig)
{
    if(SIGALRM == sig)
    {
        //printf("epoll_timer\n");
        epoll_client_systime = time(NULL);
        timer_run_every(_epoll_client);
        alarm(1);       //重新继续定时1s
    }
    return ;
}
#if 0
static int connect_server(const char host, const int port)
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
#else
static int connect_server(const char host[], const int port)
{
    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;
    //char ch = 'A';
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(host);
    address.sin_port = htons(port);
    len = sizeof(address);
    result = connect(sockfd, (struct sockaddr *)&address, len);

    if (result == -1)
    {
        //perror("oops: client1");
        //exit(1);
        return -1;
    }
    //write(sockfd, &ch, 1);
    //read(sockfd, &ch, 1);
    return sockfd;
}
#endif

static struct epoll_thread_data* get_epoll_data(struct epoll_obj* const _this, const int fd)
{
    int i;
    struct epoll_thread_data* const _data_list = (struct epoll_thread_data*)_this->data;
    struct epoll_thread_data* _data=NULL;
    //printf("[%s-%d] data:%p\n", __func__, __LINE__, _this->data);
    for(i=0; i<epoll_obj_data_size; i++)
    {
        //printf("[%s-%d] _data_list: %p\n", __func__, __LINE__, _data_list);
        _data = &_data_list[i];
        //printf("[%s-%d] i:%d\n", __func__, __LINE__, i);
        if(fd==_data->fd)
        {
            return _data;
        }
    }
    //printf("[%s-%d] \n", __func__, __LINE__);
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

//Client
static void do_epoll_client(struct epoll_obj* const _this, int listenfd)
{
    int ret;
    int sockfd;
    char buf[MAXSIZE];
    struct local_config_data* _cfg_data = (struct local_config_data*)_local_config_data->data;
    memset(buf, 0, MAXSIZE);
    _epoll_client=_this;
    epoll_client_systime = time(NULL);
    _this->epollfd = epoll_create(FDSIZE);
    printf("[%s-%d] \n", __func__, __LINE__);
    //_this->fops.add_event(_this, stdout, EPOLLOUT);
    //_this->fops.add_event(_this, STDOUT_FILENO, EPOLLOUT);
    //sockfd = connect_server("183.237.191.186", 6100);
    //printf("[%s-%d] sockfd:%d \n", __func__, __LINE__, sockfd);
    //_this->fops.add_event(_this, sockfd, EPOLLIN);
    //printf("[%s-%d] \n", __func__, __LINE__);
    signal(SIGALRM, epoll_timer); //注册安装信号
    alarm(1);       //触发定时器
    printf("[%s-%d] \n", __func__, __LINE__);
    //_this->fops.modify_event(_this, listenfd, EPOLLOUT);
    while(1)
    {
        //printf("[%s-%d] _this->fd_count:%d connect_counts:%d\n", __func__, __LINE__, _this->fd_count, _cfg_data->_vin_cfg.connect_counts);
        //if(_this->fd_count<_cfg_data->_vin_cfg.connect_counts)
        if(_this->fd_count<1)
        {
            struct epoll_thread_data* _thread_data = NULL;
            //printf("[%s-%d] \n", __func__, __LINE__);
            sockfd = connect_server("183.237.191.186", 6100);
            //printf("[%s-%d] \n", __func__, __LINE__);
            _thread_data = get_epoll_data(_this, sockfd);
            //printf("[%s-%d] _thread_data: %p\n", __func__, __LINE__, _thread_data);
            if(NULL!=_thread_data)
            {
                printf("[%s-%d] _this->fd_count:%d sockfd:%d\n", __func__, __LINE__, _this->fd_count, sockfd);
                _thread_data->_obd_obj = obd_agree_obj_yunjing.fops->constructed(&obd_agree_obj_yunjing, _thread_data->_obd_obj_buf);
                _thread_data->flag = SERVER_TYPE_OBD_YJ;
                _thread_data->_timeout = 0;//epoll_client_systime + epoll_client_timeout;
                _this->fops.add_event(_this, sockfd, EPOLLIN);
                //_this->fops.modify_event(_this, _thread_data->fd, EPOLLOUT);
            }
            else
            {
                close(sockfd);
                memset(_thread_data, 0, sizeof(struct local_config_data));
            }
        }
        //printf("[%s-%d] \n", __func__, __LINE__);
        ret = epoll_wait(_this->epollfd, _this->events, EPOLLEVENTS, -1);
        printf("[%s-%d] _this->fd_count:%d\n", __func__, __LINE__, _this->fd_count);
        //timer_run_every(_this);
        _this->handle_events(_this, _this->events, ret, listenfd, buf, sizeof(buf));
    }
    close(_this->epollfd);
}

static void handle_events_client(struct epoll_obj* const _this, struct epoll_event* const events, const int num, const int listenfd, char* const buf, const int _max_size)
{
    (void)listenfd;
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
        if(events[i].events & EPOLLIN)
        {
            int nread;
            int decode; // 解码数据
            //struct epoll_thread_data* const _thread_data = (struct epoll_thread_data*)events[i].data.ptr;
            //printf("[%s-%d ]EPOLLIN\n", __func__, __LINE__);
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
            /*int send=-1;
            _thread_data = get_epoll_data(_this, fd);
            send=request_vin(_this, _thread_data);
            if(0==send) _thread_data->_timeout = 0;*/
            //_this->fops.do_write(_this, fd, buf, strlen(buf));
            _this->fops.do_write(_this, fd, buf, 0);
        }
    }
    //_db_report.fops->insert_sql(_db_report);
    //_db_report.fops->close(_db_report);
}
static void epoll_close_client(struct epoll_obj* const _this, const int fd)
{
    struct epoll_thread_data* const _thread_data = get_epoll_data(_this, fd);
    memset(_thread_data, 0, sizeof(struct epoll_thread_data));
}
struct epoll_obj* epoll_client_init(void* const _epoll_buf, void* const data)
{
    struct epoll_obj* _epoll=NULL;
    //_epoll = epoll_obj_base.fops.constructed(&epoll_obj_base, _epoll_buf, do_epoll_listen, handle_events_listen, handle_accept_listen, NULL);
    printf("[%s-%d] data:%p\n", __func__, __LINE__, data);
    _epoll = epoll_obj_base.fops.constructed(&epoll_obj_base, _epoll_buf, do_epoll_client, handle_events_client, NULL, epoll_close_client, data);
    return _epoll;
}

