/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : obdii_dev.c
* Author             : Merafour
* Last Modified Date : 11/25/2019
* Description        : OBDII Device.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "obdii_dev.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>
#include <stdio.h>

#include "../agreement/msg_print.h"

enum obdii_status{
    OBDII_START = 0x00,
    OBDII_CONNECT = 0x01,
    OBDII_GET_VIN = 0x02,
};

#define  cmd_ack(cmd)   (cmd+0x80)

static const int client_timeout = 3;


/*
 * return socket
 * */
static int connect_init(const char host[], const int port)
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

static int obd_exit(struct obdii_dev_obj* const _this)
{
    printf("Disconnect : %d\n", _this->socket);
    close(_this->socket);
    return 0;
}

static int obd_connect(struct obdii_dev_obj* const _this, const char _host[], const int _port, struct msg_print_obj* const _print)
{
    int socket=0;
    _print->fops->print(_print, "VIN码线程第 %ld 次建立连接 ...\n", _this->connect_count++);
    socket = connect_init(_host, _port);
    if(socket>=0)
    {
        _print->fops->print(_print, "VIN码连接建立结束 host:%s port:%d fd:%d\n\n", _host, _port, socket);
        _this->socket = socket;
        _this->connect_count = 0;
        return socket;
    }
    return -1;
}

static int loop(struct obdii_dev_obj* const _this, const char _host[], const int _port, struct msg_print_obj* const _print, time_t _systime)
{
    //printf("[%s-%d] _this->status:%d\n", __func__, __LINE__, _this->status);
    switch(_this->status)
    {
    case OBDII_START:
        _this->status = OBDII_CONNECT;
        _this->timeout = _systime;
        break;
    case OBDII_CONNECT:
        if(_this->connect(_this, _host, _port, _print))
        {
            _this->status = OBDII_GET_VIN;
        }
        break;
    case OBDII_GET_VIN:
        if(_this->timeout<_systime)
        {
            // send get vin req
            //printf("[%s-%d] OBDII_GET_VIN\n", __func__, __LINE__);
            if(0==_this->get_vin(_this, _print, _systime))
            {
                //printf("[%s-%d] OBDII_GET_VIN\n", __func__, __LINE__);
                _this->status = cmd_ack(OBDII_GET_VIN);
                _this->timeout = _systime + client_timeout;
            }
            //printf("[%s-%d] OBDII_GET_VIN\n", __func__, __LINE__);
        }
        break;
    case cmd_ack(OBDII_GET_VIN):
        if(_this->timeout<_systime)
        {
            // send get vin req
            //printf("[%s-%d] cmd_ack(OBDII_GET_VIN)\n", __func__, __LINE__);
            _this->get_vin_ack(_this, _print, _systime);
            _this->status = OBDII_GET_VIN;
        }
        else
        {
            if(0==_this->get_vin_ack(_this, _print, _systime)) _this->status = OBDII_GET_VIN;
        }
        break;
    default:
        _this->status = OBDII_START;
        break;
    }
    return 0;
}

static int do_read(struct obdii_dev_obj* const _this, char* const buf, const int _max_size)
{
    int nread;
    //printf("[%s-%d] \n", __func__, __LINE__);
    nread = read(_this->socket, buf, _max_size);
    if(nread == -1)
    {
        //printf("[%s-%d] \n", __func__, __LINE__);
        perror("read error:");
        close(_this->socket);
        _this->socket = -1;
        _this->status = OBDII_CONNECT;
    }
    else if(nread == 0)
    {
        //printf("[%s-%d] \n", __func__, __LINE__);
        fprintf(stderr, "client close.\n");
        close(_this->socket);
        _this->socket = -1;
        _this->status = OBDII_CONNECT;
    }
    else
    {
        //printf("[%s-%d] \n", __func__, __LINE__);
        //pr_debug("read message is : %s", buf);
        //_this->fops.modify_event(_this, fd, EPOLLOUT);
    }
    //printf("[%s-%d] \n", __func__, __LINE__);
    return nread;
}
static int do_write(struct obdii_dev_obj* const _this, char* buf, const int _size)
{
    int nwrite;
    //nwrite = write(fd, buf, strlen(buf));
    nwrite = write(_this->socket, buf, _size);
    if(nwrite == -1)
    {
        perror("write error:");
        close(_this->socket);
        _this->socket = -1;
        _this->status = OBDII_CONNECT;
    }
    //else
        return nwrite;
    //memset(buf, 0, MAXSIZE);
}

static int get_vin(struct obdii_dev_obj* const _this, struct msg_print_obj* const _print, time_t const _systime)
{
    char wsn[32];
    char cache[512];
    const uint32_t cache_size = sizeof(cache);
    //char _vin[512];
    //const uint32_t _vsize = sizeof(_vin);
    char _msg_buf[512];
    const uint32_t _msize = sizeof(_msg_buf);
    struct yunjing_userdef _udef;
    //int ret = 0;
    int len = 0;
    //int _size = 0;
    //struct obd_agree_ofp_data _ofp_data;
    struct obd_agree_obj* const _obd_obj = _this->_obd_obj;
    struct tm *_tmp=NULL;
    //printf("[%s-%d] _this->_obd_obj:%p\n", __func__, __LINE__, _this->_obd_obj);
    memset(wsn, 0, sizeof(wsn));
    if(0==_obd_obj->fops->base->vin.req_get(wsn))
    {
        //printf("[%s-%d] \n", __func__, __LINE__);
        _tmp=localtime(&_systime);
        //printf("[%s-%d] \n", __func__, __LINE__);
        _print->fops->print(_print, "[%s-%d] request VIN SN[%d-%d-%d %02d:%02d:%02d]:[%s]\n", __func__, __LINE__, _tmp->tm_year+1900, _tmp->tm_mon+1, _tmp->tm_mday, _tmp->tm_hour, _tmp->tm_min, _tmp->tm_sec, wsn);
        memset(&_udef, 0, sizeof(struct yunjing_userdef));
        //printf("[%s-%d] \n", __func__, __LINE__);
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
        _this->write(_this, _msg_buf, len);
#if 0
        printf("[%s-%d] \n", __func__, __LINE__);
        usleep(1000*200);   // 200ms delay
        printf("[%s-%d] \n", __func__, __LINE__);
        memset(cache, 0, sizeof(cache));
        printf("[%s-%d] \n", __func__, __LINE__);
        //_size = read_threads(socket, cache, cache_size, &recv_status);
        _size = _this->read(_this, cache, cache_size);
        printf("[%s-%d] \n", __func__, __LINE__);
        if(0>=_size) // close, 重新创建连接
        {
            return -1;
        }
        printf("[%s-%d] \n", __func__, __LINE__);
        //_size = net->recv(net, cache, cache_size, _recv_timeout); // socket_read(cache, cache_size, 4000);
        //ret = decode_client(_agree_obd_yj, (const uint8_t*)cache, _size, _msg_buf, _msize, _vin, _vsize, &tlen);
        _print->fops->print(_print, "[%s-%d] cache[%d]:%s\n", __func__, __LINE__, _size, cache); fflush(stdout);
        ret = _obd_obj->fops->decode_client(_obd_obj, (uint8_t*)cache, (uint16_t)_size, _msg_buf, _msize, &_ofp_data, _print);
        //if(tlen>0) net->send(net, _buf, tlen);// csend(0, _buf, tlen);
        //if(STATUS_CLIENT_DONE==ret) // if((ERR_CLIENT_DOWN==ret) || (STATUS_CLIENT_DONE==ret))
        if((ERR_CLIENT_DOWN==ret) || (STATUS_CLIENT_DONE==ret))
        {
            _print->fops->print(_print, "[%s-%d] STATUS_VIN OK[%s]:[%s %s]\n", __func__, __LINE__, _obd_obj->sn, wsn, _ofp_data._tbuf); //fflush(stdout);
            //vin_list_insert(wsn, _vin);
            //thread_vin_request_del(wsn);
            //printf("[%s-%d] _obd_obj->sn:%s\n", __func__, __LINE__, _obd_obj->sn); fflush(stdout);
            _obd_obj->fops->base->vin.insert(_obd_obj->sn, _ofp_data._tbuf);
            _obd_obj->fops->base->vin.req_del(_obd_obj->sn);
            _obd_obj->fops->base->vin.req_del(wsn);
        }
#endif
        return 0;
    }
    //printf("[%s-%d] \n", __func__, __LINE__);
    return -1;
}

static int get_vin_ack(struct obdii_dev_obj* const _this, struct msg_print_obj* const _print, time_t const _systime)
{
    //char wsn[32];
    char cache[512];
    const uint32_t cache_size = sizeof(cache);
    //char _vin[512];
    //const uint32_t _vsize = sizeof(_vin);
    char _msg_buf[512];
    const uint32_t _msize = sizeof(_msg_buf);
    //const struct agreement_ofp* _agree_obd_yj=NULL;
    //struct yunjing_userdef _udef;
    int ret = 0;
    //int len = 0;
    int _size = 0;
    struct obd_agree_ofp_data _ofp_data;
    struct obd_agree_obj* const _obd_obj = _this->_obd_obj;
    //memset(wsn, 0, sizeof(wsn));
    memset(cache, 0, sizeof(cache));
    //printf("[%s-%d]\n", __func__, __LINE__);
    _size = _this->read(_this, cache, cache_size);
    //printf("[%s-%d] _size:%d\n", __func__, __LINE__, _size);
    if(0>=_size) // close, 重新创建连接
    {
        return -1;
    }
    //_print->fops->print(_print, "[%s-%d] cache[%d]:%s\n", __func__, __LINE__, _size, cache); fflush(stdout);
    ret = _obd_obj->fops->decode_client(_obd_obj, (uint8_t*)cache, (uint16_t)_size, _msg_buf, _msize, &_ofp_data, _print);
    if((ERR_CLIENT_DOWN==ret) || (STATUS_CLIENT_DONE==ret))
    {
        _print->fops->print(_print, "[%s-%d] STATUS_VIN OK:[%s %s]\n", __func__, __LINE__, _obd_obj->sn, _ofp_data._tbuf); //fflush(stdout);
        _obd_obj->fops->base->vin.insert(_obd_obj->sn, _ofp_data._tbuf);
        _obd_obj->fops->base->vin.req_del(_obd_obj->sn);
        //_obd_obj->fops->base->vin.req_del(_obd_obj->sn);
        _this->timeout =  _systime + client_timeout;
        return 0;
    }
    /*memset(wsn, 0, sizeof(wsn));
    printf("%s\n", _print->__stream); fflush(stdout);
    _print->fops->fflush(_print);*/
    return-2;
}

struct obdii_dev_obj* obdii_dev_obj(struct obdii_dev_obj* const _this, void* const _obj_buf)
{
    struct obdii_dev_obj _fops = {
        .obdii_dev_obj = _this->obdii_dev_obj,
                .connect = _this->connect,
                .loop = _this->loop,
                .get_vin = _this->get_vin,
                .get_vin_ack = _this->get_vin_ack,
                .exit = _this->exit,
                .read = _this->read,
                .write = _this->write,
    };
    _fops.status = 0;
    struct obdii_dev_obj* const _obj = (struct obdii_dev_obj*)_obj_buf;
    memcpy(_obj_buf, &_fops, sizeof(_fops));
    _obj->_obd_obj = obd_agree_obj_yunjing.fops->constructed(&obd_agree_obj_yunjing, _obj->_obd_obj_buf);
    _obj->_obd_obj->fops->init(_obj->_obd_obj, 0, (const uint8_t*)"IMEI1234567890ABCDEF", (const uint8_t*)"VIN0123456789ABCDEF", 2, "INFO");
    //printf("[%s-%d] _obj->_obd_obj:%p\n", __func__, __LINE__, _obj->_obd_obj);
    return _obj;
}

struct obdii_dev_obj* new_obdii_dev_obj(void* const _obj_buf)
{
    struct obdii_dev_obj _fops = {
        .obdii_dev_obj = obdii_dev_obj,
                .connect = obd_connect,
                .loop = loop,
                .get_vin = get_vin,
                .get_vin_ack = get_vin_ack,
                .exit = obd_exit,
                .read = do_read,
                .write = do_write,
    };
    //_fops._obd_obj = obd_agree_obj_yunjing.fops->constructed(&obd_agree_obj_yunjing, _fops._obd_obj_buf);
    _fops.status = 0;
    struct obdii_dev_obj* const _obj = (struct obdii_dev_obj*)_obj_buf;
    memcpy(_obj_buf, &_fops, sizeof(_fops));
    _obj->_obd_obj = obd_agree_obj_yunjing.fops->constructed(&obd_agree_obj_yunjing, _obj->_obd_obj_buf);
    _obj->_obd_obj->fops->init(_obj->_obd_obj, 0, (const uint8_t*)"IMEI1234567890ABCDEF", (const uint8_t*)"VIN0123456789ABCDEF", 2, "INFO");
    //printf("[%s-%d] _obj->_obd_obj:%p\n", __func__, __LINE__, _obj->_obd_obj);
    return _obj;
}



