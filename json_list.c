/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : json_list.h
* Author             : Merafour
* Last Modified Date : 06/15/2019
* Description        : dynamic memory allocation library.
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
#include "json_list.h"

#define  debug_log     0
#ifndef debug_log
#define  debug_log     1
#endif
//#undef   debug_log
#include <stdio.h>

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
int create_cfg(const char *path)
{
    cJSON *_root = NULL;
    cJSON *item_json = NULL;
    char *out = NULL;
    FILE* fd = NULL;

    _root = cJSON_CreateObject();
    cJSON_AddStringToObject(_root, "cJSON Version", cJSON_Version());
    cJSON_AddStringToObject(_root, "Host", "39.108.72.130");
    cJSON_AddNumberToObject(_root, "Port", 9910);
    cJSON_AddNumberToObject(_root, "SLL", 1);
    cJSON_AddItemToObject(_root, "SLL.Des", item_json = cJSON_CreateObject());
    cJSON_AddStringToObject(item_json, "0", "SSL_INFO");
    cJSON_AddStringToObject(item_json, "1", "SSL_RSA");
    cJSON_AddStringToObject(item_json, "2", "SSL_SM2");
    cJSON_AddStringToObject(_root, "CarType", "FuelType");   // 汽油车
    cJSON_AddItemToObject(_root, "CarType.Des", item_json = cJSON_CreateObject());
    cJSON_AddStringToObject(item_json, "FuelType", "汽油车");
    cJSON_AddStringToObject(item_json, "DieselType", "柴油车");
    cJSON_AddNumberToObject(_root, "GPS_Time", 10);   // 10s
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
    fd = fopen(path, "w+");  // w+ : 可读可写, 可以不存在, 必会擦掉原有内容从头写
    if(NULL==fd)
    {
        pr_debug("fopen fail!\n"); fflush(stdout);
        mem_free(out);
        fflush(stdout);
        return -2;
    }
    fwrite(out, strlen(out), 1, fd);
    fwrite("\n", 1, 1, fd);
    fflush(fd);
    fclose(fd);
    mem_free(out);
    pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    return 0;
}
int create_list(const char *path)
{
    cJSON *_root = NULL;
    cJSON *item_json = NULL;
    char *out = NULL;
    FILE* fd = NULL;
    //int i=0;

    /* Our matrix: */
    /*int numbers[3][3] =
    {
        {0, -1, 0},
        {1, 0, 0},
        {0 ,0, 1}
    };*/

    _root = cJSON_CreateObject();
    cJSON_AddStringToObject(_root, "cJSON Version", cJSON_Version());
    cJSON_AddItemToObject(_root, "Update", item_json = cJSON_CreateObject());
    cJSON_AddStringToObject(item_json, "up_0", "102905420118");
    cJSON_AddStringToObject(item_json, "up_1", "102905420128");
    cJSON_AddItemToObject(_root, "DeviceList", item_json = cJSON_CreateObject());
    cJSON_AddStringToObject(item_json, "dev_1", "102905420118");
    cJSON_AddStringToObject(item_json, "dev_2", "102905420128");
    cJSON_AddStringToObject(item_json, "dev_3", "102905420138");

    //cJSON_AddItemToObject(_root, "List", cJSON_CreateIntArray(numbers[0], 3));
    //cJSON_AddItemToArray(item_json, cJSON_CreateIntArray(numbers[0], 3));


    out = cJSON_Print(_root);
    //out = cJSON_PrintUnformatted(_root);
    cJSON_Delete(_root);
    if(NULL==out)
    {
        pr_debug("no memory, perused: %d  \n", mem_perused());  fflush(stdout);
        return -1;
    }
    //pr_debug("data write to %s  \n", json_table);  fflush(stdout);
    //out = cJSON_Print(_root);
    pr_debug("%s\n", out); fflush(stdout);
    fd = fopen(path, "w+");  // w+ : 可读可写, 可以不存在, 必会擦掉原有内容从头写
    if(NULL==fd)
    {
        pr_debug("fopen fail!\n"); fflush(stdout);
        mem_free(out);
        fflush(stdout);
        return -2;
    }
    fwrite(out, strlen(out), 1, fd);
    fwrite("\n", 1, 1, fd);
    fflush(fd);
    fclose(fd);
    mem_free(out);
    pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    return 0;
}
static char json_buf[1024*1024]; // 1MB
int json_list_add(const char *path, const char* sn, const int device)
{
    cJSON *_root = NULL;
    cJSON *node_json = NULL;
    cJSON *item_json = NULL;
    char *out = NULL;
    FILE* fd = NULL;
    long _size=0;

    fd = fopen(path, "r");  //
    if(NULL==fd)
    {
        pr_debug("fopen fail!\n"); fflush(stdout);
        return -1;
    }
    fseek(fd, 0, SEEK_END);
    _size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    pr_debug("_size %s :%ld \n", path, _size);  fflush(stdout);
    fread(json_buf, _size, 1, fd);
    fclose(fd);

    //pr_debug("data:\n%s\n", json_buf); fflush(stdout);

    _root = cJSON_Parse(json_buf);
    if(NULL == _root)  // 数据损坏
    {
        pr_debug("data bad\n"); fflush(stdout);
        return -2;
    }
    if(0==device)  // update
    {
        node_json = cJSON_GetObjectItem(_root, "Update");
    }
    else // DeviceList
    {
        node_json = cJSON_GetObjectItem(_root, "DeviceList");
    }
    if(NULL == node_json)  // 数据损坏
    {
        pr_debug("index_json bad\n"); fflush(stdout);
        cJSON_Delete(_root);
        return -3;
    }
    item_json = node_json->child;
    while(NULL!=item_json)
    {
        pr_debug("item_json : %s | %s\n", item_json->string, item_json->valuestring); fflush(stdout);
        if(NULL==item_json->next)
        {
            char str[64];
            int number=0;
            size_t i=0;
            char ch=0;
            memset(str, 0, sizeof (str));
            //sscanf(item_json->string, "%s%d", str, &number);
            for(i=0; i<strlen(item_json->string); i++)
            {
                ch = item_json->string[i];
                if((ch>='0') && (ch <= '9'))
                {
                    number = atoi(&item_json->string[i]);
                    break;
                }
                str[i] = ch;
            }
            pr_debug("item_json : %s | %d\n", str, number); fflush(stdout);
            number++;
            snprintf(str, sizeof (str)-1, "%s%d", str, number);
            cJSON_AddStringToObject(node_json, str, sn);
            break;
        }
        item_json = item_json->next;
    }

    out = cJSON_Print(_root);
    //out = cJSON_PrintUnformatted(_root);
    //pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
    cJSON_Delete(_root);
    if(NULL==out)
    {
        pr_debug("no memory, perused: %d  \n", mem_perused());  fflush(stdout);
        return -1;
    }
    //pr_debug("data write to %s  \n", json_table);  fflush(stdout);
    //out = cJSON_Print(_root);
    pr_debug("%s\n", out); fflush(stdout);
    fd = fopen(path, "w+");  // w+ : 可读可写, 可以不存在, 必会擦掉原有内容从头写
    if(NULL==fd)
    {
        pr_debug("fopen fail!\n"); fflush(stdout);
        mem_free(out);
        fflush(stdout);
        return -2;
    }
    fwrite(out, strlen(out), 1, fd);
    fwrite("\n", 1, 1, fd);
    fflush(fd);
    fclose(fd);
    mem_free(out);
    pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    return 0;
}
int json_list_del(const char *path, const char* sn, const int device)
{
    cJSON *_root = NULL;
    cJSON *node_json = NULL;
    cJSON *item_json = NULL;
    char *out = NULL;
    FILE* fd = NULL;
    long _size=0;

    fd = fopen(path, "r");  //
    if(NULL==fd)
    {
        pr_debug("fopen fail!\n"); fflush(stdout);
        return -1;
    }
    fseek(fd, 0, SEEK_END);
    _size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    pr_debug("_size %s :%ld \n", path, _size);  fflush(stdout);
    fread(json_buf, _size, 1, fd);
    fclose(fd);

    //pr_debug("data:\n%s\n", json_buf); fflush(stdout);

    _root = cJSON_Parse(json_buf);
    if(NULL == _root)  // 数据损坏
    {
        pr_debug("data bad\n"); fflush(stdout);
        return -2;
    }
    if(0==device)  // update
    {
        node_json = cJSON_GetObjectItem(_root, "Update");
    }
    else // DeviceList
    {
        node_json = cJSON_GetObjectItem(_root, "DeviceList");
    }
    if(NULL == node_json)  // 数据损坏
    {
        pr_debug("index_json bad\n"); fflush(stdout);
        cJSON_Delete(_root);
        return -3;
    }
    item_json = node_json->child;
    while(NULL!=item_json)
    {
        pr_debug("item_json : %s | %s\n", item_json->string, item_json->valuestring); fflush(stdout);
        if(0==strcmp(item_json->valuestring, sn))
        {
            cJSON_Delete(cJSON_DetachItemViaPointer(node_json, item_json));
            item_json = node_json->child;
        }
        item_json = item_json->next;
    }

    out = cJSON_Print(_root);
    //out = cJSON_PrintUnformatted(_root);
    //pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
    cJSON_Delete(_root);
    if(NULL==out)
    {
        pr_debug("no memory, perused: %d  \n", mem_perused());  fflush(stdout);
        return -1;
    }
    //pr_debug("data write to %s  \n", json_table);  fflush(stdout);
    //out = cJSON_Print(_root);
    pr_debug("%s\n", out); fflush(stdout);
    fd = fopen(path, "w+");  // w+ : 可读可写, 可以不存在, 必会擦掉原有内容从头写
    if(NULL==fd)
    {
        pr_debug("fopen fail!\n"); fflush(stdout);
        mem_free(out);
        fflush(stdout);
        return -2;
    }
    fwrite(out, strlen(out), 1, fd);
    fwrite("\n", 1, 1, fd);
    fflush(fd);
    fclose(fd);
    mem_free(out);
    pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    return 0;
}
int json_list_search(const char *path, const char* sn, const int device)
{
    cJSON *_root = NULL;
    cJSON *node_json = NULL;
    cJSON *item_json = NULL;
    FILE* fd = NULL;
    long _size=0;

    fd = fopen(path, "r");  //
    if(NULL==fd)
    {
        pr_debug("fopen fail!\n"); fflush(stdout);
        return -1;
    }
    fseek(fd, 0, SEEK_END);
    _size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    pr_debug("_size %s :%ld \n", path, _size);  fflush(stdout);
    fread(json_buf, _size, 1, fd);
    fclose(fd);

    //pr_debug("data:\n%s\n", json_buf); fflush(stdout);

    _root = cJSON_Parse(json_buf);
    if(NULL == _root)  // 数据损坏
    {
        pr_debug("data bad\n"); fflush(stdout);
        return -2;
    }
    if(0==device)  // update
    {
        node_json = cJSON_GetObjectItem(_root, "Update");
    }
    else // DeviceList
    {
        node_json = cJSON_GetObjectItem(_root, "DeviceList");
    }
    if(NULL == node_json)  // 数据损坏
    {
        pr_debug("index_json bad\n"); fflush(stdout);
        cJSON_Delete(_root);
        return -3;
    }
    item_json = node_json->child;
    while(NULL!=item_json)
    {
        pr_debug("item_json : %s | %s\n", item_json->string, item_json->valuestring); fflush(stdout);
        if(0==strcmp(item_json->valuestring, sn))
        {
            cJSON_Delete(_root);
            pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
            fflush(stdout);
            return 0;
        }
        item_json = item_json->next;
    }
    cJSON_Delete(_root);
    pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    return -1;
}

