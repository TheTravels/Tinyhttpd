/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : thread_list.c
* Author             : Merafour
* Last Modified Date : 06/15/2019
* Description        : Thread List.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include "json_list.h"
//#include "thread_list.h"
#include "thread_vin.h"
#include "../agreement/agreement.h"
#include "../agreement/obd_agree_shanghai.h"
#include "../agreement/obd_agree_fops.h"
#include "../agreement/obd_agree_yunjing.h"
#include "msg_relay.h"
#include "../config/config_data.h"


#ifndef BUILD_THREAD_VIN
#define BUILD_THREAD_VIN   0
#endif
#ifndef BUILD_SERVER_YN
#define BUILD_SERVER_YN   0
#endif

static struct vin_item vin_item_request[256];
static const uint16_t vin_item_request_size = sizeof(vin_item_request)/sizeof(vin_item_request[0]);
static uint16_t vin_item_request_index = 0;

static pthread_mutex_t vin_lock;     // 锁

//static char msg_buf[4096];
static char log_path[128];

/*static int msg_print(char *__stream, const size_t __n, const char *__format, ...)
{
    char* text=NULL;
    size_t _size=0;
    va_list ap;
    _size = strlen(__stream);
    if(_size>=__n) return -1;
    text = &__stream[_size];
    va_start(ap, __format);
    //vprintf(__format, ap);
    //snprintf(text, sizeof (text), __format, ap);
    vsprintf(text, __format, ap);
    va_end(ap);
    return 0;
}
static int log_write_to_file(char *__stream, const size_t __n, const char *_path)
{
    FILE* fd=NULL;
    fd = fopen(_path, "ab");
    if(NULL!=fd)
    {
        fwrite(__stream, __n, 1, fd);
        fflush(fd);
        fclose(fd);
    }
    printf("%s", __stream); fflush(stdout);
    return 0;
}*/

void thread_vin_init(const uint16_t _port)
{
    //time_t timep;
    //struct tm *_tmp=NULL;
    //time(&timep);
    //_tmp=localtime(&timep);
    pthread_mutex_init (&vin_lock, NULL);
    //printf("head_free: 0x%08X 0x%08X 0x%08X \n", _free, _free->next, _free->prev); fflush(stdout);
    //pthread_mutex_lock (&vin_lock);
    //pthread_mutex_unlock (&vin_lock);
    memset(&vin_item_request, 0, sizeof(vin_item_request));
    memcpy(vin_item_request[0].sn, "440303ZA0CK90N0224", 18);
    memcpy(vin_item_request[1].sn, "440303ZA0CK90N0210", 18);
    memset(log_path, 0, sizeof (log_path));
    //snprintf(log_path, sizeof (log_path)-1, "log/thread_vin_%d_%d-%d-%d.txt", _port, _tmp->tm_year+1900, _tmp->tm_mon+1, _tmp->tm_mday);
    snprintf(log_path, sizeof (log_path)-1, "thread_vin_%d", _port);
    //printf("log/thread_vin_%d_%d-%d-%d.txt", _port, _tmp->tm_year+1900, _tmp->tm_mon+1, _tmp->tm_mday); fflush(stdout);
    //printf("时间： %d-%d-%d %02d:%02d:%02d", _tmp->tm_year+1900, _tmp->tm_mon+1, _tmp->tm_mday, _tmp->tm_hour, _tmp->tm_min, _tmp->tm_sec); fflush(stdout);
}
#if 0
static void __thread_vin_request_add(const char* const sn)
{
    struct vin_item *_item;
    //pthread_mutex_lock (&vin_lock);
    _item = &vin_item_request[vin_item_request_index++];
    vin_item_request_index = vin_item_request_index%vin_item_request_size;
    memset(_item, 0, sizeof(struct vin_item));
    memcpy(_item->sn, sn, strlen(sn));
    //pthread_mutex_unlock (&vin_lock);
}

void thread_vin_request_add(const char* const sn)
{
    pthread_mutex_lock (&vin_lock);
    __thread_vin_request_add(sn);
    __thread_vin_request_add(sn);
    pthread_mutex_unlock (&vin_lock);
}
void thread_vin_request_del(const char* const sn)
{
    uint16_t index;
    struct vin_item *_item;
    pthread_mutex_lock (&vin_lock);
    for(index=0; index<vin_item_request_size; index++)
    {
        _item = &vin_item_request[index];
        if(0==strcmp(sn, _item->sn))
        {
            memset(_item, 0, sizeof(struct vin_item));
        }
    }
    pthread_mutex_unlock (&vin_lock);
}
int thread_vin_request_get(char _sn[])
{
    uint16_t index;
    struct vin_item *_item;
    pthread_mutex_lock (&vin_lock);
    for(index=0; index<vin_item_request_size; index++)
    {
        _item = &vin_item_request[index];
        if(strlen(_item->sn)>=8)
        {
            memcpy(_sn, _item->sn, strlen(_item->sn));
            pthread_mutex_unlock (&vin_lock);
            return 0;
        }
    }
    pthread_mutex_unlock (&vin_lock);
    return -1;
}
#endif
//extern void csend(const int sockfd, const void *buf, const uint16_t len);
extern int read_threads(int sock, char *buf, int size, int *status);
//static const char vin_host[] = "yjobdc.cloudscape.net.cn";
#if (0==BUILD_SERVER_YN)
//static const char vin_host[] = "183.237.191.186";
#else
//static const char vin_host[] = "192.168.0.80";
#endif
//static const int vin_port = 6100;
static int do_read(int const fd, char* const buf, const int _max_size)
{
    int nread;
    nread = read(fd, buf, _max_size);
    if(nread == -1)
    {
        perror("read error:");
        close(fd);
    }
    else if(nread == 0)
    {
        fprintf(stderr, "client close.\n");
        close(fd);
    }
    else
    {
        //pr_debug("read message is : %s", buf);
        //_this->fops.modify_event(_this, fd, EPOLLOUT);
    }
    return nread;
}
static int do_write(int fd, char* buf, const int _size)
{
    int nwrite;
    //nwrite = write(fd, buf, strlen(buf));
    nwrite = write(fd, buf, _size);
    if(nwrite == -1)
    {
        perror("write error:");
        close(fd);
    }
    //else
        return nwrite;
    //memset(buf, 0, MAXSIZE);
}
// 获取 VIN码线程
void thread_get_vin(void *arg)
{
    (void)arg;
    struct local_config_data* _cfg_data = (struct local_config_data*)_local_config_data->data;
    int socket=0;
    int i=0;
    char wsn[32];
    char cache[512];
    const uint32_t cache_size = sizeof(cache);
    //char _vin[512];
    //const uint32_t _vsize = sizeof(_vin);
    char _msg_buf[512];
    const uint32_t _msize = sizeof(_msg_buf);
    //const struct agreement_ofp* _agree_obd_yj=NULL;
    struct yunjing_userdef _udef;
    //int recv_status=1;
    //int tlen = 0;
    int ret = 0;
    int len = 0;
    int _size = 0;
    char __stream[1024*30] = "\0";
    char _print_obj_buf[sizeof(struct msg_print_obj)];
    struct msg_print_obj* _print_obj = _msg_obj.fops->constructed(&_msg_obj, _print_obj_buf, log_path, __stream, sizeof(__stream));
    /*{
        .fops = &_msg_print_fops,
        .__stream = __stream,
        .__n = sizeof(__stream),
    };*/
    struct obd_agree_ofp_data _ofp_data;
    //const int _get_flag=1;
    time_t timep, tnow;
    struct tm *_tmp=NULL;
    char _obd_obj_buf[sizeof(struct obd_agree_obj)];
    struct obd_agree_obj* const _obd_obj = obd_agree_obj_yunjing.fops->constructed(&obd_agree_obj_yunjing, _obd_obj_buf);
    //_agree_obd_yj = create_agree_obd_yunjing();
    _obd_obj->fops->init(_obd_obj, 0, (const uint8_t*)"IMEI1234567890ABCDEF", (const uint8_t*)"VIN0123456789ABCDEF", 2, "INFO");
    //memset(msg_buf, 0, sizeof (msg_buf));
    time(&tnow);
    timep = tnow;
    _print_obj->fops->init(_print_obj);
    //if(_get_flag)
    {
connect:
        _print_obj->fops->print(_print_obj, "VIN码连接建立中 host:%s port:%d\n", _cfg_data->_vin_cfg.host, _cfg_data->_vin_cfg.port);
        for(i=0; i<1000; i++)
        {
            _print_obj->fops->print(_print_obj, "VIN码线程第 %ld 次建立连接 ...\n", i);
            socket = relay_init(_cfg_data->_vin_cfg.host, _cfg_data->_vin_cfg.port);
            if(socket>=0) break;
            usleep(1000*100);   // 100ms delay
        }
        _print_obj->fops->print(_print_obj, "VIN码连接建立结束 host:%s port:%d fd:%d\n\n", _cfg_data->_vin_cfg.host, _cfg_data->_vin_cfg.port, socket);
    }
    time(&tnow);
    _print_obj->fops->print(_print_obj, "VIN码线程启动[%ds] TCP: %d\n\n", tnow-timep, socket);
    printf("%s\n", _print_obj->__stream); fflush(stdout);
    _print_obj->fops->fflush(_print_obj);
    //memset(msg_buf, 0, sizeof (msg_buf));
    timep = tnow;
    while(socket>=0)
    {
        memset(wsn, 0, sizeof(wsn));
        //memset(msg_buf, 0, sizeof (msg_buf));
        _print_obj->fops->init(_print_obj);
        //while(0==thread_vin_request_get(wsn))
        while(0==_obd_obj->fops->base->vin.req_get(wsn))
        {
            _tmp=localtime(&timep);
            _print_obj->fops->print(_print_obj, "[%s-%d] request VIN SN[%d-%d-%d %02d:%02d:%02d]:[%s]\n", __func__, __LINE__, _tmp->tm_year+1900, _tmp->tm_mon+1, _tmp->tm_mday, _tmp->tm_hour, _tmp->tm_min, _tmp->tm_sec, wsn);
            memset(&_udef, 0, sizeof(struct yunjing_userdef));
            memset(cache, 0, sizeof(cache));
            //memset(_vin, 0, sizeof(_vin));
            memset(_msg_buf, 0, sizeof(_msg_buf));
            _udef.type_msg = USERDEF_YJ_QUERY_VIN; // 请求 VIN码
            memcpy(_udef.msg, wsn, 18);
            //len = userdef_encode_yj(&_udef, cache, cache_size);   // user def
            len = _obd_obj->fops->userdef_encode(_obd_obj, &_udef, cache, cache_size);
            len = _obd_obj->fops->base->pack.encode(_obd_obj, cache, len, (uint8_t*)_msg_buf, _msize); // len = _agree->encode(cache, len, msg_buf, sizeof (msg_buf));
            //printf("login pack encode len : %d\n", len); fflush(stdout);
            //csend(0, msg_buf, len);
            //net->send(net, _msg_buf, len); memcpy(repeat_buf, _msg_buf, len); repeat_buf_len = len;
            memset(_obd_obj->sn, 0, sizeof(_obd_obj->sn));
            memcpy(_obd_obj->sn, wsn, strlen(wsn));
            do_write(socket, _msg_buf, len);
            usleep(1000*200);   // 200ms delay
            memset(cache, 0, sizeof(cache));
            //_size = read_threads(socket, cache, cache_size, &recv_status);
            _size = do_read(socket, cache, cache_size);
            if(0>=_size) // close, 重新创建连接
            {
                socket = -1;
#if BUILD_THREAD_VIN
                goto connect;
#endif
            }
            //_size = net->recv(net, cache, cache_size, _recv_timeout); // socket_read(cache, cache_size, 4000);
            //ret = decode_client(_agree_obd_yj, (const uint8_t*)cache, _size, _msg_buf, _msize, _vin, _vsize, &tlen);
            _print_obj->fops->print(_print_obj, "[%s-%d] cache[%d]:%s\n", __func__, __LINE__, _size, cache); fflush(stdout);
            ret = _obd_obj->fops->decode_client(_obd_obj, (uint8_t*)cache, (uint16_t)_size, _msg_buf, _msize, &_ofp_data, _print_obj);
            //if(tlen>0) net->send(net, _buf, tlen);// csend(0, _buf, tlen);
            //if(STATUS_CLIENT_DONE==ret) // if((ERR_CLIENT_DOWN==ret) || (STATUS_CLIENT_DONE==ret))
            if((ERR_CLIENT_DOWN==ret) || (STATUS_CLIENT_DONE==ret))
            {
                _print_obj->fops->print(_print_obj, "[%s-%d] STATUS_VIN OK[%s]:[%s %s]\n", __func__, __LINE__, _obd_obj->sn, wsn, _ofp_data._tbuf); //fflush(stdout);
                //vin_list_insert(wsn, _vin);
                //thread_vin_request_del(wsn);
                //printf("[%s-%d] _obd_obj->sn:%s\n", __func__, __LINE__, _obd_obj->sn); fflush(stdout);
                _obd_obj->fops->base->vin.insert(_obd_obj->sn, _ofp_data._tbuf);
                _obd_obj->fops->base->vin.req_del(_obd_obj->sn);
                _obd_obj->fops->base->vin.req_del(wsn);
            }
            memset(wsn, 0, sizeof(wsn));
            //log_write_to_file(msg_buf, strlen(msg_buf), log_path);
            //memset(msg_buf, 0, sizeof (msg_buf));
            printf("%s\n", _print_obj->__stream); fflush(stdout);
            _print_obj->fops->fflush(_print_obj);
#if (0==BUILD_THREAD_VIN)
            break;
#endif
            //sleep(1);
        }
        usleep(1000*200);   // 200ms delay
        //sleep(1);
        time(&tnow);
        if(tnow>=(timep+60))
        {
            timep = tnow;
            _tmp=localtime(&timep);
            _print_obj->fops->print(_print_obj, "当前时间： %d-%d-%d %02d:%02d:%02d\n", _tmp->tm_year+1900, _tmp->tm_mon+1, _tmp->tm_mday, _tmp->tm_hour, _tmp->tm_min, _tmp->tm_sec); //fflush(stdout);
        }
        //log_write_to_file(msg_buf, strlen(msg_buf), log_path);
        if(strlen(_print_obj->__stream)>0)
        {
            printf("%s\n", _print_obj->__stream); fflush(stdout);
            _print_obj->fops->fflush(_print_obj);
        }
    }
    if(socket>=0) close(socket);
    _print_obj->fops->print(_print_obj, "VIN码线程退出 TCP: %d\n\n", socket);
    //log_write_to_file(msg_buf, strlen(msg_buf), log_path);
    printf("%s\n", _print_obj->__stream); fflush(stdout);
    _print_obj->fops->fflush(_print_obj);
}






