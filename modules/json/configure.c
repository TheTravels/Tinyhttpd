/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : configure.h
* Author             : Merafour
* Last Modified Date : 06/15/2019
* Description        : JSON List.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#include "cJSON.h"
#include "mem_malloc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../obd/json_list.h"

#define  debug_log     0
#ifndef debug_log
#define  debug_log     1
#endif
//#undef   debug_log
#include <stdio.h>

#define DATA_ARRAY   1   // json 数组

//#ifdef debug_log
#if debug_log
#define pr_debug(fmt, ...) printf(fmt, ##__VA_ARGS__); fflush(stdout);
#else
#define pr_debug(fmt, ...) ;
#endif

/**
{
    "cJSON Version":	"1.7.12",
    "Host":	"39.108.72.130",
    "Port":	9910,
    "SLL":	1,
    "CarType":	"FuelType ",
    "GPS_Time":	10,
    "Car_Protocol":	0,
    "CAN_bps":	500000
}
 */

int create_cfg_file(const char *path, char *json)
{
    FILE* fd = NULL;
    fd = fopen(path, "w+");  // w+ : 可读可写, 可以不存在, 必会擦掉原有内容从头写
    if(NULL==fd)
    {
        pr_debug("fopen fail!\n"); fflush(stdout);
        fflush(stdout);
        return -2;
    }
    fwrite(json, strlen(json), 1, fd);
    fwrite("\n", 1, 1, fd);
    fflush(fd);
    fclose(fd);
    fflush(stdout);
    return 0;
}
int create_configure(const char *path, const struct config_data* const cfg)
{
    cJSON *_root = NULL;
    cJSON *item_json = NULL;
    char *out = NULL;

    _root = cJSON_CreateObject();
    cJSON_AddStringToObject(_root, "cJSON Version", cJSON_Version());
    cJSON_AddStringToObject(_root, "Host", cfg->Host);
    cJSON_AddNumberToObject(_root, "Port", cfg->Port);
    cJSON_AddNumberToObject(_root, "SLL", cfg->ssl);
    cJSON_AddItemToObject(_root, "SLL.Des", item_json = cJSON_CreateObject());
    cJSON_AddStringToObject(item_json, "0", "SSL_INFO");
    cJSON_AddStringToObject(item_json, "1", "SSL_RSA");
    cJSON_AddStringToObject(item_json, "2", "SSL_SM2");
    cJSON_AddStringToObject(_root, "CarType", cfg->CarType);   // 汽油车
    cJSON_AddItemToObject(_root, "CarType.Des", item_json = cJSON_CreateObject());
    cJSON_AddStringToObject(item_json, "FuelType", "汽油车");
    cJSON_AddStringToObject(item_json, "DieselType", "柴油车");
    cJSON_AddNumberToObject(_root, "WiFi", cfg->wifi);
    cJSON_AddItemToObject(_root, "WiFi.Des", item_json = cJSON_CreateObject());
    cJSON_AddStringToObject(item_json, "0", "WiFi Turn On");
    cJSON_AddStringToObject(item_json, "1", "WiFi Turn Off");
    cJSON_AddNumberToObject(_root, "GPS_Time", cfg->GPS_Time);   // 10s
    cJSON_AddNumberToObject(_root, "Car_Protocol", 0);   //
    cJSON_AddNumberToObject(_root, "CAN_bps", 0);   // 0 kbps
    cJSON_AddNumberToObject(_root, "CAN_PIN", 0);

    out = cJSON_Print(_root);
    //out = cJSON_PrintUnformatted(_root);
    cJSON_Delete(_root);
    if(NULL==out)
    {
        printf("no memory, perused: %d  \n", mem_perused());  fflush(stdout);
        return -1;
    }
    //pr_debug("data write to %s  \n", json_table);  fflush(stdout);
    //out = cJSON_Print(_root);
    pr_debug("%s\n", out); fflush(stdout);
    create_cfg_file(path, out);
    mem_free(out);
    pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    return 0;
}
int create_cfg(const char *path)
{
    const char Host[] = "39.108.72.130";
    const char CarType[] = "FuelType";
    struct config_data cfg;
    memset(&cfg, 0, sizeof (cfg));
    memcpy(cfg.Host, Host, sizeof (Host));
    cfg.Port = 9910;
    cfg.ssl = 1;
    memcpy(cfg.CarType, CarType, sizeof (CarType));
    cfg.GPS_Time = 10;
    return create_configure(path, &cfg);
}



