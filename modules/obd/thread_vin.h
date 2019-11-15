/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : thread_list.h
* Author             : Merafour
* Last Modified Date : 10/08/2019
* Description        : Thread List.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/

#ifndef THREAD_VIN_H
#define THREAD_VIN_H

#include "list.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <time.h>

extern void thread_vin_init(const uint16_t _port);
extern void thread_vin_request_add(const char* const sn);
extern void thread_get_vin(void *arg);

#ifdef __cplusplus
}
#endif


#endif // THREAD_VIN_H
