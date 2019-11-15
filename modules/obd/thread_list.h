/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : thread_list.h
* Author             : Merafour
* Last Modified Date : 06/15/2019
* Description        : Thread List.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/

#ifndef THREAD_LIST_H
#define THREAD_LIST_H

#include "list.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <time.h>

#define  DEVICE_CACHE_SIZE   512

#define SOCKET_TYPE_DEVICE    0    // 设备连接
#define SOCKET_TYPE_TRUNK     1    // 中继
#define SOCKET_TYPE_NULL      2    // 空
struct device_list{   // 设备列表
    struct list_head list;
    struct list_head list_work; // 工作链表
    time_t last_time;      // 上一次收到数据的时间
    uint16_t busy;         // 忙标志
    uint16_t socket_type;  // 连接类型
    char buf[1024];
    size_t buf_pos;
    int crc;
    char UTC[6];
    int socket;       // TCP / IP
    int relay_fd;     // msg relay,转发文件描述符
    int save_log;
    int type;         // 协议类型
    int protocol; // 协议类型
    char sn[32];      // 序列号
    char VIN[32];     // 车辆识别码
    uint8_t cache[DEVICE_CACHE_SIZE];      // 数据缓存
    uint16_t len;     // 数据长度
    uint16_t write;   // 数据更新计数
    uint16_t fw_update;
    uint16_t fw_flag;
    //struct list_head msg_list;  // 消息链表
};

extern void thread_list_init(void);
extern struct device_list* get_thread_free(void);
extern void online_thread_add(struct device_list* dev);
extern void online_thread_for_each(void* client, void (*call)(void* client, const struct device_list* const device, const uint8_t pack[], const uint16_t _psize));
extern void online_thread_free(struct device_list* dev);

#ifdef __cplusplus
}
#endif


#endif // THREAD_LIST_H
