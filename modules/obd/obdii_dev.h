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
#ifndef _OBDII_DEV_H_
#define _OBDII_DEV_H_

#ifdef __cplusplus
extern "C"
{
#endif
	
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include "../agreement/obd_agree_yunjing.h"
#include "../agreement/obd_agree_fops.h"

struct obdii_dev_obj {
    struct obdii_dev_obj* (*const obdii_dev_obj)(struct obdii_dev_obj* const _this, void* const _obj_buf);
    int (*const connect)(struct obdii_dev_obj* const _this, const char _host[], const int _port, struct msg_print_obj* const _print);
    int (*const loop)(struct obdii_dev_obj* const _this, const char _host[], const int _port, struct msg_print_obj* const _print, time_t _systime);
    int (*const get_vin)(struct obdii_dev_obj* const _this, struct msg_print_obj* const _print, time_t _systime);
    int (*const get_vin_ack)(struct obdii_dev_obj* const _this, struct msg_print_obj* const _print, time_t const _systime);
    int (*const read)(struct obdii_dev_obj* const _this, char* const buf, const int _max_size);
    int (*const write)(struct obdii_dev_obj* const _this, char* const buf, const int _max_size);
    int (*const exit)(struct obdii_dev_obj* const _this);

    struct obd_agree_obj* _obd_obj;
    char _obd_obj_buf[sizeof(struct obd_agree_obj)];
    time_t timeout;
    int socket;
    int connect_count;
    int status;
};

struct obdii_dev_obj_buf{
    struct obdii_dev_obj* _dev_obj;
    char buf[sizeof(struct obdii_dev_obj)];
};


struct obdii_dev_obj* new_obdii_dev_obj(void* const _obj_buf);


#ifdef __cplusplus
}
#endif

#endif /* _OBDII_DEV_H_ */

