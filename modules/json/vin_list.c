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

//union vin_data{
//    char *data[2];
//    struct car{
//        char* sn;
//        char* vin;
//    }uint;
//};
static const char* vin_list[][2] = {
{"102906420029",   "LGAX3BG42J1026005"},
{"102906420019",   "LGAX2BG46G1030246"},
{"102906420019",   "LGAX4C357F3006359"},
{"102906420019",   "LGAX4C353E8044902"},
{"102906420019",   "LGAX2BG41E1073583"},
{"102906420019",   "LZGJLGM14EX125632"},
{"102906420019",   "LZGJLGM10EX126521"},
{"102906420019",   "LZZ1CCND6GD221128"},
{"102906420019",   "LZGJDGN1XHX029925"},
{"102906420019",   "LZGJDTM15EX003190"},
{"102906420019",   "LC1AJLBC2E0001735"},
{"102906420019",   "LZGJDGN11HX029926"},
{"102906420019",   "LZGJLGM16EX124997"},
{"102906420019",   "LGAX4C353F3006360"},
{"102906420019",   "LGAX40351F3006356"},
{"102906420019",   "LGAX4C352H3041992"},
{"102906420019",   "LGAX4C351J3029418"},
{"102906420019",   "LGAX4C351J3029418"},
{"102906420019",   "LFWSRXSJ1G1E56629"},
{"102906420019",   "LFNCRUMX3H1E36912"},
{"102906420019",   "LFNAHULX1J1E02694"},
{"102906420019",   "LGAX2BG48G1030247"},
{"102906420019",   "LGAX4C359E8095563"},
};

static struct vin_item vin_item_list[128];
static const uint16_t vin_item_list_size = sizeof(vin_item_list)/sizeof(vin_item_list[0]);
static uint16_t vin_item_list_index = 0;

static int _vin_list_load(const char *path, struct vin_item* const _list, const uint16_t _list_size)
{
    cJSON *_root = NULL;
    cJSON *node = NULL;
    const char* data=NULL;
    uint16_t index;
    //char* out = NULL;
    _root = read_json_file(path);
    if(NULL == _root)  // 数据损坏
    {
        pr_debug("data bad\n"); fflush(stdout);
        return -1;
    }
    /*out = cJSON_Print(_root);
    printf("json:%s\n", out);  fflush(stdout);
    mem_free(out);*/
    node = _root->child;
    if(NULL!=node) node = node->next;
    if(NULL!=node) node = node->next;
    index = 0;
    while(NULL!=node)
    {
#if 0
        //data = cJSON_GetStringValue(node);
        data = cJSON_GetArrayItem(node, 0)->valuestring;
        //printf("node SN:%s [%d] VIN:%s \n", node->string, cJSON_GetArraySize(node), cJSON_GetArrayItem(node, 0)->valuestring); fflush(stdout);
        if(0==strcmp(sn, data))
        {
            data = cJSON_GetArrayItem(node, 1)->valuestring;
            memcpy(vin, data, strlen(data));
            return 0;
        }
#endif
        //printf("node SN:%s [%d] VIN:%s \n", node->string, cJSON_GetArraySize(node), cJSON_GetArrayItem(node, 0)->valuestring); fflush(stdout);
        data = cJSON_GetArrayItem(node, 0)->valuestring;
        memcpy(_list[index].sn, data, strlen(data));
        data = cJSON_GetArrayItem(node, 1)->valuestring;
        memcpy(_list[index].vin, data, strlen(data));
        index++;
        index = index%_list_size;
        node = node->next;
    }
    return -1;
}
int vin_list_load(const char *path)
{
    uint16_t index;
    const struct vin_item* _list;
    memset(vin_item_list, 0, sizeof(vin_item_list));
    vin_item_list_index = 0;
    _vin_list_load(path, vin_item_list, vin_item_list_size);
    for(index=0; index<vin_item_list_size; index++)
    {
        _list = &vin_item_list[index];
        if(strlen(_list->vin)<=0) continue;
        printf("vin_item_list[%d]:\t%s \t\t%s\n", index, _list->sn, _list->vin);  fflush(stdout);
    }
    return 0;
}
int vin_list_search(const char* const sn, char vin[])
{
    uint16_t index;
    const struct vin_item* _list;
    if(strlen(sn)<8) return -1;
    for(index=0; index<vin_item_list_size; index++)
    {
        _list = &vin_item_list[index];
        if(0==strcmp(sn, _list->sn))
        {
            memcpy(vin, _list->vin, strlen(_list->vin));
            return 0;
        }
    }
    return -2;
}
int vin_list_insert(const char* const sn, const char* const vin)
{
    uint16_t index;
    struct vin_item _item;
    if(strlen(sn)<8) return -1;
    if(strlen(vin)<17) return -2;
    index = vin_item_list_index;
    memset(&_item, 0, sizeof(_item));
    memcpy(_item.sn, sn, strlen(sn));
    memcpy(_item.vin, vin, strlen(vin));
    memcpy(&vin_item_list[index], &_item, sizeof(struct vin_item));
    index++;
    vin_item_list_index = index%vin_item_list_size;
    return 0;
}

static int create_file(const char *path, char *json)
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
static int create_json(const char *const path)
{
    cJSON *_root = NULL;
    char *out = NULL;
    int index=0;
    char name[128];
    static const char* list_des[2] = {"序列号", "VIN"};
    static const int list_size = sizeof (vin_list)/sizeof (vin_list[0]);

    _root = cJSON_CreateObject();
    cJSON_AddStringToObject(_root, "cJSON Version", cJSON_Version());
    cJSON_AddItemToObject(_root, "List.Des", cJSON_CreateStringArray(list_des, 2));
    for(index=0; index<list_size; index++)
    {
        memset(name, 0, sizeof (name));
        snprintf(name, sizeof (name)-1, "List_%d", index+1);
        //cJSON_CreateStringArray(list[index].data, 2);
        cJSON_AddItemToObject(_root, name, cJSON_CreateStringArray((vin_list[index]), 2));
    }

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
    create_file(path, out);
    mem_free(out);
    pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    return 0;
}
int create_vin_search(const char *path, const char* const sn, char vin[])
{
    cJSON *_root = NULL;
    cJSON *node = NULL;
    const char* data=NULL;
//    char* out = NULL;
    _root = read_json_file(path);
    if(NULL == _root)  // 数据损坏
    {
        pr_debug("data bad\n"); fflush(stdout);
        return -1;
    }
//    out = cJSON_Print(_root);
//    printf("json:%s\n", out);  fflush(stdout);
//    mem_free(out);
    node = _root->child;
    node = node->next;
    while(NULL!=node)
    {
        //data = cJSON_GetStringValue(node);
        data = cJSON_GetArrayItem(node, 0)->valuestring;
        //printf("node SN:%s [%d] VIN:%s \n", node->string, cJSON_GetArraySize(node), cJSON_GetArrayItem(node, 0)->valuestring); fflush(stdout);
        if(0==strcmp(sn, data))
        {
            data = cJSON_GetArrayItem(node, 1)->valuestring;
            memcpy(vin, data, strlen(data));
            return 0;
        }
        node = node->next;
    }
    return -1;
}
int create_vin_list(const char *path)
{
    return create_json(path);
}



