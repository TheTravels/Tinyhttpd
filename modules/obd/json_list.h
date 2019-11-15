/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : json_list.h
* Author             : Merafour
* Last Modified Date : 06/15/2019
* Description        : JSON List.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#ifndef _JSON_LIST_H_
#define _JSON_LIST_H_

#ifdef __cplusplus
extern "C"
{
#endif
	
#include <stdint.h>
#include <stddef.h>
#include "cJSON.h"

enum json_list_node{
    JSON_LIST_UP     = 0x00,
    JSON_LIST_DEV    = 0x01,
    JSON_LIST_MODEL  = 0x02,
    JSON_LIST_PUSH   = 0x03,
    JSON_LIST_HIT    = 0x04,
    JSON_LIST_COMPLE = 0x05,
};
#define CFG_WiFi_Turn_On     1
#define CFG_WiFi_Turn_Off    0
#define CFG_SSL_INFO         0
#define CFG_SSL_RSA          1
#define CFG_SSL_SM2          2
struct config_data{
    char Host[64];
    int Port;
    int ssl;
    char CarType[64];
    int GPS_Time;
    int wifi;
};
struct device_node{
    char name[64];
    char SN[64];
    char Model[64];
    char Firmware[64];
    char Config[64];
};

struct vin_item{
    char sn[32];
    char vin[32];
};

extern int create_vin_list(const char *path);
extern int create_cfg_file(const char *path, char *json);
extern int create_cfg(const char *path);
extern int create_configure(const char *path, const struct config_data* const cfg);
extern int create_list(const char *path);
extern int json_list_add_node(const char *path, const enum json_list_node device, const struct device_node* const _node);
extern int json_list_add(const char *path, const char* sn, const enum json_list_node device);
extern int json_list_del(const char *path, const char* sn, const enum json_list_node device);
//extern int json_list_search(const char *path, const char* sn, const enum json_list_node device);
extern int json_list_search(const char *path, const char* _search, const enum json_list_node device, char firmware[], char config[]);
//extern int json_list_search_model(const char *path, const char* model, const enum json_list_node device, char firmware[], char config[]);
extern int json_list_read(const char *path, const int _index, const enum json_list_node device, struct device_node* const _node);
//extern char* get_json_buf(void);
extern cJSON* read_json_file(const char *const path);
extern int create_vin_search(const char *path, const char* const sn, char vin[]);
extern int vin_list_load(const char *path);
extern int vin_list_search(const char* const sn, char vin[]);
extern int vin_list_insert(const char* const sn, const char* const vin);


#ifdef __cplusplus
}
#endif

#endif /* _JSON_LIST_H_ */

