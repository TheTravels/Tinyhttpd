#include "carinfo.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "cJSON/cJSON.h"
#include "cJSON/mem_malloc.h"

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

/*
 * 车架号 车辆品牌及型号 发动机厂商 发动机型号 排放阶段 车辆类型 车牌号
 */

const char* CarListPath="./upload/Car.list";

static const char* ListTitle[7] = {"车架号", "车辆品牌及型号", "发动机厂商", "发动机型号", "排放阶段", "车辆类型", "车牌号"};
#if 1
static const char* CarInfoList[][7] = {
  //{"车架号",                 "车辆品牌及型号",        "发动机厂商",                 "发动机型号",             "排放阶段",   "车辆类型"   ,  "车牌号"},
    {"LGAX3BG42J1026005",     "东风天锦",             "DFH5180XRYBX2DV",          "康明斯 ISB180 50" ,     "国五"   ,    "厢式货车"  ,   "沪E L1570"},
    {"LGAX2BG46G1030246",     "东风天锦",             "DFC5160XRYBX1A" ,          "康明斯 ISB180 40" ,     "国四"   ,    "厢式货车"  ,   "沪D Q6111"},
    {"LGAX4C357F3006359",     "东风天龙",             "DFL1253AX1B"    ,          "康明斯 ISDe245 40",     "国四"   ,    "厢式货车"  ,   "沪D F9665"},
    {"LGAX4C353E8044902",     "东风天龙",             "DFL5253XXYAX1B" ,          "康明斯 ISB220 40" ,     "国四"   ,    "厢式货车"  ,   "沪D E0139"},
    {"LGAX2BG41E1073583",     "东风天锦",             "DFL5160XXYBX1A" ,          "康明斯 ISB180 40" ,     "国四"   ,    "厢式货车"  ,   "沪D E0021"},
    {"LZGJLGM14EX125632",     "陕汽" ,               "SX4186GN361"    ,          "潍柴 WP10.290E40" ,     "国四"   ,    "集卡"     ,    "沪D B0100"},
    {"LZGJLGM10EX126521",     "陕汽" ,               "SX4186GN361"    ,          "潍柴 WP10.290E40" ,     "国四"   ,    "集卡"     ,    "沪D B2748"},
    {"LZZ1CCND6GD221128",     "重汽豪沃",             "ZZ4187N3617E1"  ,          "重汽 D10.31-50"  ,      "国五"   ,    "集卡"     ,    "沪D R7937"},
    {"LZGJDGN1XHX029925",     "陕汽",                "SX4180MB1"      ,          "潍柴 WP7.300E51"  ,     "国五"   ,    "集卡"     ,    "沪E E0116"},
    {"LZGJDTM15EX003190",     "陕汽",                "SX4186TL351"    ,          "潍柴 WP10.270E40" ,     "国四"   ,    "集卡"     ,    "沪D 82092"},
    {"LC1AJLBC2E0001735",     "徐工",                "NXG4180D4KA"    ,          "潍柴 WP10.300E40" ,     "国四"   ,    "集卡"     ,    "沪D C3263"},
    {"LZGJDGN11HX029926",     "陕汽", 			    "SX4180MB1"      ,          "潍柴 WP7.300E51"  ,     "国五"   ,    "集卡"     ,    "沪E E9166"},
    {"LZGJLGM16EX124997",     "陕汽", 			    "SX4186GN361"    ,          "潍柴 WP10.290E40" ,     "国四"   ,    "集卡"     ,    "沪D A1497"},
    {"LGAX4C353F3006360",     "东风", 			    "DFL1253AX1B"    ,          "康明斯 ISDe245 40",     "国四"   ,    "厢式货车"  ,   "沪D G0219"},
    {"LGAX40351F3006356",     "东风", 			    "DFL1253AX1B"    ,          "康明斯 ISDe245 40",     "国四"   ,    "厢式货车"  ,   "沪D G0163"},
    {"LGAX4C352H3041992",     "东风", 			    "DFH1250AXV"     ,          "康明斯 ISD245 50" ,     "国五"   ,    "厢式货车"  ,   "沪E R9519"},
    {"LGAX4C351J3029418",     "东风", 			    "DFH5250XRYAXV"  ,          "东风 DDi75S245-50",     "国五"   ,    "厢式货车"  ,   "沪E A9366"},
    {"LGAX4C351J3029418",     "东风", 			    "DFH5250XRYAXV"  ,          "东风 DDi75S245-50",     "国五"   ,    "厢式货车"  ,   "沪E K5630"},
    {"LFWSRXSJ1G1E56629",     "解放", 			    "CA4260P66K24T1A1E6",       "锡柴 CA6DM2-46E61",     "国五"   ,    "集卡"     ,    "沪F D5130"},
    {"LFNCRUMX3H1E36912",     "解放", 			    "CA5250CCYP63K1L6T3A1E5",   "锡柴 CA6DK1-28E5" ,     "国五"   ,    "厢式货车"  ,   "沪E Q1076"},
    {"LFNAHULX1J1E02694",     "解放", 			    "CA5180XXYP62K1L7L5" ,      "锡柴 CA4DK1-22E51",     "国五"   ,    "厢式货车"  ,   "沪E G7200"},
    {"LGAX2BG48G1030247",     "东风", 			    "DFC5160XRYBX1A"     ,      "康明斯 ISB180 40" ,     "国四"   ,    "厢式货车"  ,   "沪D Q7200"},
    {"LGAX4C359E8095563",     "东风", 			    "DFL5253XLCAX1B"     ,      "康明斯 ISC83 245E40A",  "国四"   ,    "厢式货车"  ,   "沪D D8558"},
};
#else
static const char* CarInfoList[][7] = {
    //{"车架号",                 "车辆品牌及型号", "发动机厂商",                 "发动机型号",             "排放阶段",   "车辆类型"   ,  "车牌号"},
    {"LGAX3BG42J1026005",     "DF",             "DFH5180XRYBX2DV",          "ISB180 50" ,        "GW"   ,    "XSHC"  ,   "E L1570"},
    {"LGAX2BG46G1030246",     "DF",             "DFC5160XRYBX1A" ,          "ISB180 40" ,        "GS"   ,    "XSHC"  ,   "D Q6111"},
    {"LGAX4C357F3006359",     "DF",             "DFL1253AX1B"    ,          "ISDe245 40",        "GS"   ,    "XSHC"  ,   "D F9665"},
    {"LGAX4C353E8044902",     "DF",             "DFL5253XXYAX1B" ,          "ISB220 40" ,        "GS"   ,    "XSHC"  ,   "D E0139"},
    {"LGAX2BG41E1073583",     "DF",             "DFL5160XXYBX1A" ,          "ISB180 40" ,        "GS"   ,    "XSHC"  ,   "D E0021"},
    {"LZGJLGM14EX125632",     "SQ" ,            "SX4186GN361"    ,          "WP10.290E40" ,      "GS"   ,    "JK"    ,    "D B0100"},
    {"LZGJLGM10EX126521",     "SQ" ,            "SX4186GN361"    ,          "WP10.290E40" ,      "GS"   ,    "JK"    ,    "D B2748"},
    {"LZZ1CCND6GD221128",     "ZH",             "ZZ4187N3617E1"  ,          "D10.31-50"  ,       "GW"   ,    "JK"    ,    "D R7937"},
    {"LZGJDGN1XHX029925",     "SQ",             "SX4180MB1"      ,          "WP7.300E51"  ,      "GW"   ,    "JK"    ,    "E E0116"},
    {"LZGJDTM15EX003190",     "SQ",             "SX4186TL351"    ,          "WP10.270E40" ,      "GS"   ,    "JK"    ,    "D 82092"},
    {"LC1AJLBC2E0001735",     "XG",             "NXG4180D4KA"    ,          "WP10.300E40" ,      "GS"   ,    "JK"    ,    "D C3263"},
    {"LZGJDGN11HX029926",     "SQ", 	        "SX4180MB1"      ,          "WP7.300E51"  ,      "GW"   ,    "JK"    ,    "E E9166"},
    {"LZGJLGM16EX124997",     "SQ", 			"SX4186GN361"    ,          "WP10.290E40" ,      "GS"   ,    "JK"    ,    "D A1497"},
    {"LGAX4C353F3006360",     "DF", 			"DFL1253AX1B"    ,          "ISDe245 40",        "GS"   ,    "XSHC"  ,   "D G0219"},
    {"LGAX40351F3006356",     "DF", 			"DFL1253AX1B"    ,          "ISDe245 40",        "GS"   ,    "XSHC"  ,   "D G0163"},
    {"LGAX4C352H3041992",     "DF", 			"DFH1250AXV"     ,          "ISD245 50" ,        "GW"   ,    "XSHC"  ,   "E R9519"},
    {"LGAX4C351J3029418",     "DF", 			"DFH5250XRYAXV"  ,          "DDi75S245-50",      "GW"   ,    "XSHC"  ,   "E A9366"},
    {"LGAX4C351J3029418",     "DF", 			"DFH5250XRYAXV"  ,          "DDi75S245-50",      "GW"   ,    "XSHC"  ,   "E K5630"},
    {"LFWSRXSJ1G1E56629",     "JF", 			"CA4260P66K24T1A1E6",       "CA6DM2-46E61",      "GW"   ,    "JK"    ,    "F D5130"},
    {"LFNCRUMX3H1E36912",     "JF", 			"CA5250CCYP63K1L6T3A1E5",   "CA6DK1-28E5" ,      "GW"   ,    "XSHC"  ,   "E Q1076"},
    {"LFNAHULX1J1E02694",     "JF", 			"CA5180XXYP62K1L7L5" ,      "CA4DK1-22E51",      "GW"   ,    "XSHC"  ,   "E G7200"},
    {"LGAX2BG48G1030247",     "DF", 			"DFC5160XRYBX1A"     ,      "ISB180 40" ,        "GS"   ,    "XSHC"  ,   "D Q7200"},
    {"LGAX4C359E8095563",     "DF", 			"DFL5253XLCAX1B"     ,      "ISC83 245E40A",     "GS"   ,    "XSHC"  ,   "D D8558"},
};
#endif
static char ListData[7][128];

static void create_node(cJSON *node, const char *node_name, const char *const title[], const char *const data[], const int _size)
{
    int i =0;
    cJSON *item_json = NULL;
    cJSON_AddItemToObject(node, node_name, item_json = cJSON_CreateObject());
    for(i=0; i<_size; i++)
    {
        cJSON_AddStringToObject(item_json, title[i], data[i]);
    }
}

int CarInfoSearch(const char* path, const char* VIN, char data[])
{
#if 0
    int _size = sizeof (CarInfoList)/sizeof (CarInfoList[0]);
#else
    cJSON *_root = NULL;
    cJSON *node_json = NULL;
    cJSON *item_json = NULL;
    FILE* fd = NULL;
    int fond = 0;
    static char json_buf[1024*50];
    long _size = 0;

    fd = fopen(path, "r");
    if(NULL==fd)
    {
        pr_debug("fopen %s fail!\n", path); fflush(stdout);
        fflush(stdout);
        return -2;
    }
    fseek(fd, 0, SEEK_END);
    _size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    pr_debug("_size %s :%ld \n", path, _size);  fflush(stdout);
    memset(json_buf, 0, sizeof (json_buf));
    fread(json_buf, _size, 1, fd);
    fclose(fd);

    _root = cJSON_Parse(json_buf);
    if(NULL == _root)  // 数据损坏
    {
        pr_debug("data bad\n"); fflush(stdout);
        return -1;
    }
    node_json = _root->child;
    fond = 0;
    memset(ListData, 0, sizeof(ListData));
    // 遍历
    while(NULL!=node_json)
    {
        item_json = node_json->child;
        //printf("node: %s | %s \n", node_json->string, node_json->valuestring);
        //printf("node: %s\n", node_json->string); fflush(stdout);
        while(NULL!=item_json)
        {
            int i=0;
            pr_debug("item: %s | %s \n", item_json->string, item_json->valuestring);
            fflush(stdout);
            for(i=0; i<7; i++)
            {
                if(0==strcmp(ListTitle[i], item_json->string))
                {
                    if(0==strcmp(VIN, item_json->valuestring))
                    {
                        //printf("item: %s | %s \n", item_json->string, item_json->valuestring);
                        fond = 1;
                    }
                    pr_debug("item: %s | %s \n", item_json->string, item_json->valuestring);
                    memset(ListData[i], 0, sizeof(ListData[i]));
                    memcpy(ListData[i], item_json->valuestring, strlen(item_json->valuestring));
                    break;
                }
            }
            pr_debug("item: %s | %s \n", item_json->string, item_json->valuestring);
            item_json = item_json->next;
        }
        if(1==fond) break;
        node_json = node_json->next;
    }
    cJSON_Delete(_root);
    pr_debug("memory perused...1: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    if(1==fond)
    {
        //memset(data, 0, sizeof (ListData));
        //memset(data, 0, 128*7);
        memcpy(data, ListData, sizeof (ListData));
        return 0;
    }
    pr_debug("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
#endif
    return  -1;
}
int CreateCarInfo(const char* path)
{
    cJSON *_root = NULL;
    char name[128];
    int i=0;
    int _size = sizeof (CarInfoList)/sizeof (CarInfoList[0]);
    char *out = NULL;
    FILE* fd = NULL;

    _root = cJSON_CreateObject();
    cJSON_AddStringToObject(_root, "cJSON Version", cJSON_Version());
    for(i=0; i<_size; i++)
    {
        memset(name, 0, sizeof (name));
        snprintf(name, sizeof (name)-1, "Car_%d", i+1);
        create_node(_root, name, ListTitle, CarInfoList[i], 7);
    }

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
    //pr_debug("%s\n", out); fflush(stdout);
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
