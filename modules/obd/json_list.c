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
#include "cJSON.h"
#include "mem_malloc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "json_list.h"
#include "cJSON/cJSON.h"

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

static const char* json_SN = "SerialNumber";
static const char* json_Model = "Model";
static const char* json_Firmware = "Firmware";
static const char* json_Config = "Config";
static const char Model[] = "OBDII-4G";
static const char Firmware[] = "OBDII-4G.bin";
static const char Config[]   = "OBDII-4G.cfg";
static const char json_Update[] = "Update";
static const char json_DeviceList[] = "DeviceList";
static const char json_HitList[] = "The Hit List";
static const char json_Push[] = "Push";
static const char json_Compel[] = "Compel";
static const char* def_up[] = {
    "102903420018",
    "102906420248",
    "102906420198",
    "102906420258",
    "102906420148",
    "102906420288",
    "102906420208",
    "102906420298",
    "102906420218",
    "102906420238",
    "102906420268",
    "102906420028",
    "102906420278",
    "102906420168",
    "102906420188",
    "102906420138",
    "102906420078",
    "102906420058",
    "102906420228",
    "102905620128",
};

static void create_node(cJSON *node, const char *node_name, const char *model, const char *fw, const char *cfg, const char *sn)
{
#if (0==DATA_ARRAY)
    cJSON *item_json = NULL;
    cJSON_AddItemToObject(node, node_name, item_json = cJSON_CreateObject());
    cJSON_AddStringToObject(item_json, json_SN, sn);
    cJSON_AddStringToObject(item_json, json_Model, model);
    cJSON_AddStringToObject(item_json, json_Firmware, fw);
    cJSON_AddStringToObject(item_json, json_Config, cfg);
#else
    const char* List[4] = {
        sn,
        model,
        fw,
        cfg,
    };
    cJSON_AddItemToObject(node, node_name, cJSON_CreateStringArray(List, 4));
#endif
}
//static void create_node_dev(cJSON *node, const struct device_node* const device)
//{
//    cJSON *item_json = NULL;
//    cJSON_AddItemToObject(node, device->name, item_json = cJSON_CreateObject());
//    cJSON_AddStringToObject(item_json, "SerialNumber", device->SN);
//    cJSON_AddStringToObject(item_json, "Model", device->Model);
//    cJSON_AddStringToObject(item_json, "Firmware", device->Firmware);
//    cJSON_AddStringToObject(item_json, "Config", device->Config);
//}
#if (0==DATA_ARRAY)
char* get_node_sn(cJSON *node)
{
    cJSON *item_json = NULL;
    item_json = cJSON_GetObjectItem(node, "SerialNumber");
    if(NULL==item_json) return NULL;
    return item_json->valuestring;
}
static int node_sn_cmp(cJSON *node, const char *sn)
{
    cJSON *item_json = NULL;
    item_json = cJSON_GetObjectItem(node, "SerialNumber");
    if(NULL==item_json) return -1;
    return strcmp(item_json->valuestring, sn);
}
#endif

int create_list(const char *path)
{
    cJSON *_root = NULL;
    cJSON *node_json = NULL;
    //cJSON *item_json = NULL;
    char *out = NULL;
    FILE* fd = NULL;
    int i=0;
    char name[32];
    char fw[128];
    long _size = sizeof(def_up)/sizeof (def_up[0]);

    /* Our matrix: */
    /*int numbers[3][3] =
    {
        {0, -1, 0},
        {1, 0, 0},
        {0 ,0, 1}
    };*/
    const char* HitList[] = {
        "102906420248",
        "102906420198",
        "102906420258",
        "102906420148",
        "102906420288",
    };

    _root = cJSON_CreateObject();
    cJSON_AddStringToObject(_root, "cJSON Version", cJSON_Version());
    cJSON_AddStringToObject(_root, json_Push, "0");
    cJSON_AddItemToObject(_root, "Push.Des", node_json = cJSON_CreateObject());
    cJSON_AddStringToObject(node_json, "0", "服务器不推送更新");
    cJSON_AddStringToObject(node_json, "1", "服务器主动推送更新");
    //cJSON_AddStringToObject(_root, json_Compel, "0");
    create_node(_root, json_Compel, Model, Firmware, Config, "0");
    cJSON_AddItemToObject(_root, "Compel.Des", node_json = cJSON_CreateObject());
    cJSON_AddStringToObject(node_json, "0", "强制更新开启");
    cJSON_AddStringToObject(node_json, "1", "强制更新关闭");
    cJSON_AddItemToObject(_root, json_HitList, cJSON_CreateStringArray(HitList, 5));
    create_node(_root, "Model.Des", json_Model, json_Firmware, json_Config, json_SN);
    cJSON_AddItemToObject(_root, json_Model, node_json = cJSON_CreateObject());
    create_node(node_json, "model_0", "Model1", "Firmware1.bin", "Config1.cfg", "000000000000");
    create_node(node_json, "model_1", "Model2", "Firmware2.bin", "Config1.cfg", "000000000000");
    create_node(node_json, "model_3", Model, Firmware, Config, "000000000000");
    create_node(_root, "Up.Des", json_Model, json_Firmware, json_Config, json_SN);
    cJSON_AddItemToObject(_root, json_Update, node_json = cJSON_CreateObject());
    //create_node(node_json, "up_0", Model, Firmware, Config, "102905420118");
    //create_node(node_json, "up_1", Model, Firmware, Config, "102905420128");
    i=0;
    for(i=0; i<_size; i++)
    {
        memset(name, 0, sizeof (name));
        snprintf(name, sizeof (name)-1, "up_%d", i);
        memset(fw, 0, sizeof (fw));
        snprintf(fw, sizeof (fw)-1, "fw_%s_%s", def_up[i], Firmware);
        //printf("FW:%s\n", fw); fflush(stdout);
        create_node(node_json, name, Model, fw, Config, def_up[i]);
    }
    create_node(_root, "Dev.Des", json_Model, json_Firmware, json_Config, json_SN);
    cJSON_AddItemToObject(_root, json_DeviceList, node_json = cJSON_CreateObject());
    create_node(node_json, "dev_1", Model, Firmware, Config, "102905420118");
    create_node(node_json, "dev_2", Model, Firmware, Config, "102905420128");
    create_node(node_json, "dev_3", Model, Firmware, Config, "102905420138");
    create_node(node_json, "dev_4", Model, Firmware, Config, "102905420148");

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
//char* get_json_buf(void)
//{
//    memset(json_buf, 0, sizeof (json_buf));
//    return json_buf;
//}
cJSON* read_json_file(const char *const path)
{
    cJSON *_root = NULL;
    FILE* fd = NULL;
    long _size=0;

    fd = fopen(path, "r");  //
    if(NULL==fd)
    {
        pr_debug("fopen fail!\n"); fflush(stdout);
        return NULL;
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
        return NULL;
    }
    return _root;
}
cJSON* read_file(const char *path, cJSON **node_json, const enum json_list_node device)
{
    cJSON *_root = NULL;
#if 0
    FILE* fd = NULL;
    long _size=0;

    fd = fopen(path, "r");  //
    if(NULL==fd)
    {
        pr_debug("fopen fail!\n"); fflush(stdout);
        return NULL;
    }
    fseek(fd, 0, SEEK_END);
    _size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    pr_debug("_size %s :%ld \n", path, _size);  fflush(stdout);
    fread(json_buf, _size, 1, fd);
    fclose(fd);

    //pr_debug("data:\n%s\n", json_buf); fflush(stdout);

    _root = cJSON_Parse(json_buf);
#else
    _root = read_json_file(path);
#endif
    if(NULL == _root)  // 数据损坏
    {
        pr_debug("data bad\n"); fflush(stdout);
        return NULL;
    }
    *node_json = cJSON_GetObjectItem(_root, json_HitList);
    //printf("HitList: %s %d %s\n", (*node_json)->string, cJSON_GetArraySize(*node_json), cJSON_GetArrayItem(*node_json, 0)->valuestring);
    if(JSON_LIST_UP==device)  // update
    {
        *node_json = cJSON_GetObjectItem(_root, json_Update);
    }
    else if(JSON_LIST_MODEL == device)
    {
        *node_json = cJSON_GetObjectItem(_root, json_Model);
    }
    else if(JSON_LIST_COMPLE==device)
    {
        *node_json = cJSON_GetObjectItem(_root, json_Compel);
    }
    else if(JSON_LIST_PUSH==device)
    {
        *node_json = cJSON_GetObjectItem(_root, json_Push);
    }
    // 黑名单列表
    else if(JSON_LIST_HIT==device)
    {
        *node_json = cJSON_GetObjectItem(_root, json_HitList);
    }
    else // DeviceList
    {
        *node_json = cJSON_GetObjectItem(_root, json_DeviceList);
    }
    if(NULL == *node_json)  // 数据损坏
    {
        pr_debug("index_json bad\n"); fflush(stdout);
        cJSON_Delete(_root);
        return NULL;
    }
    return _root;
}
int write_file(const char *path, cJSON* _root)
{
    char *out = NULL;
    FILE* fd = NULL;

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
    return 0;
}
int json_list_add_node(const char *path, const enum json_list_node device, const struct device_node* const _node)
{
    cJSON *_root = NULL;
    cJSON *node_json = NULL;
    cJSON *item_json = NULL;
    const char model_sn[] = "0000000000000000";

    _root = read_file(path, &node_json, device);
    if(NULL == _root)  // 数据损坏
    {
        pr_debug("data bad\n"); fflush(stdout);
        return -2;
    }
//    printf("add_node SN: %s  \n", _node->SN);  fflush(stdout);
//    printf("add_node Model: %s  \n", _node->Model);  fflush(stdout);
//    printf("add_node Config: %s  \n", _node->Config);  fflush(stdout);
//    printf("add_node Firmware: %s  \n", _node->Firmware);  fflush(stdout);
    item_json = node_json->child;
    if(NULL==item_json)
    {
        if(JSON_LIST_DEV==device) create_node(node_json, "dev_0", _node->Model, _node->Firmware, _node->Config, _node->SN);
        if(JSON_LIST_MODEL==device) create_node(node_json, "model_0", _node->Model, _node->Firmware, _node->Config, model_sn);
        else create_node(node_json, "up_0", _node->Model, _node->Firmware, _node->Config, _node->SN);
    }
    while(NULL!=item_json)
    {
#if (0==DATA_ARRAY)
        pr_debug("item_json[%d] : %s | %s\n", device, item_json->string, get_node_sn(item_json)); fflush(stdout);
#else
        pr_debug("item_json[%d] : %s | %s\n", device, item_json->string, cJSON_GetArrayItem(item_json, 0)->valuestring); fflush(stdout);
#endif
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
            //cJSON_AddStringToObject(node_json, str, sn);
            if(JSON_LIST_MODEL == device)
            {
                create_node(node_json, str, _node->Model, _node->Firmware, _node->Config, model_sn);
            }
            else
            {
                create_node(node_json, str, _node->Model, _node->Firmware, _node->Config, _node->SN);
            }
            //create_node_dev(node_json, _node);
            break;
        }
        item_json = item_json->next;
    }

    write_file(path, _root);
    pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    return 0;
}
int json_list_add(const char *path, const char* sn, const enum json_list_node device)
{
    struct device_node _node;
    memset(&_node, 0, sizeof (_node));
    memcpy(_node.SN, sn, strlen(sn));
    memcpy(_node.Model, Model, strlen(Model));
    memcpy(_node.Firmware, Firmware, strlen(Firmware));
    memcpy(_node.Config, Config, strlen(Config));

    return json_list_add_node(path, device, &_node);
}
int json_list_del(const char *path, const char* sn, const enum json_list_node device)
{
    cJSON *_root = NULL;
    cJSON *node_json = NULL;
    cJSON *item_json = NULL;

    //pr_debug("data:\n%s\n", json_buf); fflush(stdout);

    _root = read_file(path, &node_json, device);
    if(NULL == _root)  // 数据损坏
    {
        pr_debug("data bad\n"); fflush(stdout);
        return -2;
    }
    item_json = node_json->child;
    while(NULL!=item_json)
    {
#if (0==DATA_ARRAY)
        pr_debug("item_json : %s | %s\n", item_json->string, get_node_sn(item_json)); fflush(stdout);
        //if(0==strcmp(item_json->valuestring, sn))
        if(0==node_sn_cmp(item_json, sn))
        {
            cJSON_Delete(cJSON_DetachItemViaPointer(node_json, item_json));
            item_json = node_json->child;
            continue;
        }
#else
        const char* data=NULL;
        data = cJSON_GetArrayItem(item_json, 0)->valuestring; // SN
        if(0==strcmp(data, sn))
        {
            cJSON_Delete(cJSON_DetachItemViaPointer(node_json, item_json));
            item_json = node_json->child;
            continue;
        }
#endif
        item_json = item_json->next;
    }

    write_file(path, _root);
    pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    return 0;
}
int json_list_search(const char *path, const char* _search, const enum json_list_node device, char firmware[], char config[])
{
    cJSON *_root = NULL;
    cJSON *node_json = NULL;
    cJSON *item_json = NULL;
    //cJSON *data_json = NULL;
    int find=0;

    _root = read_file(path, &node_json, device);
    if(NULL == _root)  // 数据损坏
    {
        pr_debug("data bad\n"); fflush(stdout);
        return -2;
    }
//    char* out = cJSON_Print(_root);
//    printf("JSON:\n%s", out);
//    mem_free(out);
    item_json = node_json->child;
    find = 0;
    while(NULL!=item_json)
    {
#if (0==DATA_ARRAY)
        cJSON *data_json = NULL;
        pr_debug("item_json : %s | %s\n", item_json->string, get_node_sn(item_json)); fflush(stdout);
        //if(0==strcmp(item_json->valuestring, sn))
        data_json = item_json->child;
        while(NULL!=data_json) // 遍历链表
        {
            if(0==strcmp(json_SN, data_json->string))
            {
                if(0==strcmp(data_json->valuestring, sn)) find = 1;
            }
            if(0==strcmp(json_Firmware, data_json->string)) memcpy(firmware, data_json->valuestring, strlen(data_json->valuestring));
            if(0==strcmp(json_Config, data_json->string)) memcpy(config, data_json->valuestring, strlen(data_json->valuestring));
            data_json = data_json->next;
        }
#else
        const char* data=NULL;
        pr_debug("HitList: %s %d %s\n", (item_json)->string, cJSON_GetArraySize(item_json), cJSON_GetArrayItem(item_json, 0)->valuestring);
        //printf("item_json[%d | %d]: %s \n", device, cJSON_GetArraySize(node_json), (node_json)->string);
        if((JSON_LIST_UP==device) || (JSON_LIST_DEV==device) || (JSON_LIST_MODEL==device))
        {
            if((JSON_LIST_UP==device) || (JSON_LIST_DEV==device))
            {
                data = cJSON_GetArrayItem(item_json, 0)->valuestring; // SN
                if(0==strcmp(data, _search)) find = 1;
            }
            if(JSON_LIST_MODEL==device)
            {
                data = cJSON_GetArrayItem(item_json, 1)->valuestring; // Model
                if(0==strcmp(data, _search)) find = 1;
            }
            if(1==find)
            {
                data = cJSON_GetArrayItem(item_json, 2)->valuestring; // Firmware
                //printf("Firmware1: %s %s\n", firmware, data);
                if(NULL!=firmware) memcpy(firmware, data, strlen(data));
                //printf("Firmware2: %s \n", firmware);
                data = cJSON_GetArrayItem(item_json, 3)->valuestring; // Config
                if(NULL!=config) memcpy(config, data, strlen(data));
                break;
            }
        }
        // 黑名单列表
        if(JSON_LIST_HIT==device)
        {
            int _size = cJSON_GetArraySize(node_json);
            int i=0;
            //printf("HitList: %s %d %s\n", (node_json)->string, cJSON_GetArraySize(node_json), cJSON_GetArrayItem(node_json, 0)->valuestring);
            for(i=0; i<_size; i++) // 遍历
            {
                data = cJSON_GetArrayItem(node_json, i)->valuestring;
                if(0==strcmp(data, _search)) find = 1;
            }
        }
#endif
        if(1==find)
        {
//            cJSON_Delete(_root);
//            pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
//            fflush(stdout);
//            return 0;
            break;
        }
        item_json = item_json->next;
    }
    // 主动推送, 0&1
    if(JSON_LIST_PUSH==device)
    {
        //printf("Node : %s %s %d\n", node_json->string, node_json->valuestring, node_json->valueint);
        if('1'==node_json->valuestring[0]) find = 1;
        if(1==node_json->valueint) find = 1;
    }
    // 强制更新, 0&1
    if(JSON_LIST_COMPLE==device)
    {
        const char* data=NULL;
        data = cJSON_GetArrayItem(node_json, 0)->valuestring;
        //printf("Node : %s %s %d\n", node_json->string, node_json->valuestring, node_json->valueint);
        if('1'==data[0])
        {
            find = 1;
            data = cJSON_GetArrayItem(node_json, 2)->valuestring; // Firmware
            if(NULL!=firmware) memcpy(firmware, data, strlen(data));
            data = cJSON_GetArrayItem(node_json, 3)->valuestring; // Config
            if(NULL!=config) memcpy(config, data, strlen(data));
        }
    }
    if(NULL!=_root) cJSON_Delete(_root);
    pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    if(1==find) return 0;
    return -1;
}
#if 0
int json_list_search_model(const char *path, const char* model, const enum json_list_node device, char firmware[], char config[])
{
    cJSON *_root = NULL;
    cJSON *node_json = NULL;
    cJSON *item_json = NULL;
    //cJSON *data_json = NULL;
    int find=0;

    _root = read_file(path, &node_json, device);
    if(NULL == _root)  // 数据损坏
    {
        pr_debug("data bad\n"); fflush(stdout);
        return -2;
    }
    item_json = node_json->child;
    find = 0;
    while(NULL!=item_json)
    {
#if (0==DATA_ARRAY)
        cJSON *data_json = NULL;
        pr_debug("item_json : %s | %s\n", item_json->string, get_node_sn(item_json)); fflush(stdout);
        //if(0==strcmp(item_json->valuestring, sn))
        data_json = item_json->child;
        while(NULL!=data_json) // 遍历链表
        {
            if(0==strcmp(json_Model, data_json->string))
            {
                if(0==strcmp(data_json->valuestring, model)) find = 1;
            }
            if((0==strcmp(json_Firmware, data_json->string)) && (NULL!=firmware)) memcpy(firmware, data_json->valuestring, strlen(data_json->valuestring));
            if((0==strcmp(json_Config, data_json->string)) && (NULL!=config)) memcpy(config, data_json->valuestring, strlen(data_json->valuestring));
            data_json = data_json->next;
        }
#else
        const char* data=NULL;
        pr_debug("HitList: %s %d %s\n", (item_json)->string, cJSON_GetArraySize(item_json), cJSON_GetArrayItem(item_json, 1)->valuestring);
        data = cJSON_GetArrayItem(item_json, 1)->valuestring; // Model
        if(0==strcmp(data, model)) find = 1;
        data = cJSON_GetArrayItem(item_json, 2)->valuestring; // Firmware
        if(NULL!=firmware) memcpy(firmware, data, strlen(data));
        data = cJSON_GetArrayItem(item_json, 3)->valuestring; // Config
        if(NULL!=config) memcpy(config, data, strlen(data));
#endif
        if(1==find)
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
#endif
int json_list_read(const char *path, const int _index, const enum json_list_node device, struct device_node* const _node)
{
    cJSON *_root = NULL;
    cJSON *node_json = NULL;
    cJSON *item_json = NULL;
    //char* out=NULL;
    int index=0;

    pr_debug("read path:%s\n", path); fflush(stdout);
    _root = read_file(path, &node_json, device);
    if(NULL == _root)  // 数据损坏
    {
        pr_debug("data bad\n"); fflush(stdout);
        return 0;
    }
//    out = cJSON_Print(_root);
//    printf("out:%s\n", out);
//    mem_free(out);
//    out = cJSON_Print(node_json);
//    printf("node_json:%s\n", out);
//    mem_free(out);
    item_json = node_json->child;
//    out = cJSON_Print(item_json);
//    printf("item:%s\n", out);
//    mem_free(out);
    while(NULL!=item_json)
    {
        index++;
        //printf("item_json:%08X [%d | %d] : %s | %s\n", item_json, _index, index, item_json->string, get_node_sn(item_json)); fflush(stdout);
        //if(0==strcmp(item_json->valuestring, sn))
        if(_index==index)
        {
#if 0
            cJSON *data_json = item_json->child;
            memset(_node, 0, sizeof (struct device_node));
            while(NULL!=data_json)
            {
                pr_debug("data_json : %s | %s\n", data_json->string, data_json->valuestring); fflush(stdout);
                if(0==strcmp(json_SN, data_json->string)) memcpy(_node->SN, data_json->valuestring, strlen(data_json->valuestring));
                if(0==strcmp(json_Model, data_json->string)) memcpy(_node->Model, data_json->valuestring, strlen(data_json->valuestring));
                if(0==strcmp(json_Firmware, data_json->string)) memcpy(_node->Firmware, data_json->valuestring, strlen(data_json->valuestring));
                if(0==strcmp(json_Config, data_json->string)) memcpy(_node->Config, data_json->valuestring, strlen(data_json->valuestring));
                data_json = data_json->next;
            }
#else
            // "Up.Des":	["SerialNumber", "Model", "Firmware", "Config"],
            const char *data = cJSON_GetArrayItem(item_json, 0)->valuestring;
            memcpy(_node->SN, data, strlen(data));
            data = cJSON_GetArrayItem(item_json, 1)->valuestring;
            memcpy(_node->Model, data, strlen(data));
            data = cJSON_GetArrayItem(item_json, 2)->valuestring;
            memcpy(_node->Firmware, data, strlen(data));
            data = cJSON_GetArrayItem(item_json, 3)->valuestring;
            memcpy(_node->Config, data, strlen(data));
#endif
        }
        item_json = item_json->next;
    }
    cJSON_Delete(_root);
    pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    return index;
}

