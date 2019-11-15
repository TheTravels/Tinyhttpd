/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : config_data.h
* Author             : Merafour
* Last Modified Date : 11/15/2019
* Description        : 加载配置文件并解析.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/

#ifndef CONFIG_DATA_H_
#define CONFIG_DATA_H_

#include <stdint.h>
#include "config_load.h"

#ifdef __cplusplus
extern "C" {
#endif

// server
struct host_listening {
    char host[64];
    uint16_t port;
};
// MYSQL
struct mysql_cfg{
    char host[64];
    uint16_t port;
    char user[64];
    char passwd[64];
    char db[64];
};
// 数据转发
struct data_transfer_cfg{
    char host[64];
    uint16_t port;
    char format[64];
    char turn_on[64];
    uint16_t _format;
    uint16_t _turn_on;
};
// 获取VIN配置
struct get_vin_cfg{
    char host[64];
    uint16_t port;
    char turn_on[64];
    uint16_t _turn_on;
    uint16_t connect_counts;
};
struct local_config_data{
    struct host_listening		local_listen;
    struct host_listening		local_listenCXX;
    struct host_listening		device_cfg;
    struct host_listening       local_list[20];  // 进程数
    struct mysql_cfg            _mysql_cfg;
    // 数据转发
    struct data_transfer_cfg _data_transfer_cfg;
    // 获取VIN配置
    struct get_vin_cfg _vin_cfg;
    char               vinList[64];
    uint16_t           nCpuPhtreadCounts_;
    uint16_t		   nCfgPthreadCounts_;
};

extern void local_config_data_init(void);
extern struct config_load_obj* _local_config_data;

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_DATA_H_ */
