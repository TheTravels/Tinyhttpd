/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : device_thread.c
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
#include "obdii_dev.h"

#define obdii_dev_list_size  (128)
static struct obdii_dev_obj_buf obdii_dev_list[obdii_dev_list_size];

/*seconds: the seconds; mseconds: the micro seconds*/
extern void setTimer(int seconds, int mseconds);
// 获取 VIN码线程
void thread_get_vin(void *arg)
{
    (void)arg;
    static time_t _client_systime = 0;
    struct local_config_data* _cfg_data = (struct local_config_data*)_local_config_data->data;
    int socket=0;
    int i=0;
    char __stream[1024*30] = "\0";
    char _print_obj_buf[sizeof(struct msg_print_obj)];
    struct msg_print_obj* _print_obj = _msg_obj.fops->constructed(&_msg_obj, _print_obj_buf, "log_path", __stream, sizeof(__stream));
    //const int _get_flag=1;
    //char _obd_obj_buf[sizeof(struct obd_agree_obj)];
    //struct obd_agree_obj* const _obd_obj = obd_agree_obj_yunjing.fops->constructed(&obd_agree_obj_yunjing, _obd_obj_buf);
    //_agree_obd_yj = create_agree_obd_yunjing();
    //_obd_obj->fops->init(_obd_obj, 0, (const uint8_t*)"IMEI1234567890ABCDEF", (const uint8_t*)"VIN0123456789ABCDEF", 2, "INFO");
    memset(obdii_dev_list, 0, sizeof(obdii_dev_list));
    printf("[%s-%d]\n", __func__, __LINE__);
    for(i=0; i<obdii_dev_list_size; i++)
    {
        obdii_dev_list[i]._dev_obj = new_obdii_dev_obj(obdii_dev_list[i].buf);
    }
    _print_obj->fops->init(_print_obj);
    printf("[%s-%d]\n", __func__, __LINE__);
    while(1)
    {
        struct obdii_dev_obj_buf* _dev = NULL;
        _client_systime = time(NULL);
        for(i=0; i<_cfg_data->_vin_cfg.connect_counts; i++)
        {
            _print_obj->fops->init(_print_obj);
            //printf("[%s-%d]\n", __func__, __LINE__);
            _dev = &obdii_dev_list[i];
            _dev->_dev_obj->loop(_dev->_dev_obj, _cfg_data->_vin_cfg.host, _cfg_data->_vin_cfg.port, _print_obj, _client_systime);
            //_print_obj->fops->fflush(_print_obj);
            printf("%s", _print_obj->__stream); fflush(stdout);
        }
        setTimer(1, 0);
    }
    _print_obj->fops->print(_print_obj, "VIN码线程退出 TCP: %d\n\n", socket);
    printf("%s\n", _print_obj->__stream); fflush(stdout);
    _print_obj->fops->fflush(_print_obj);
}





