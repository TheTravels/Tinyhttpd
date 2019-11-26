/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : obd_agree_fops.c
* Author             : Merafour
* Last Modified Date : 11/12/2019
* Description        : OBD agreement obj define.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#include "obd_agree_fops.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdarg.h>

#include <time.h>
#include "agreement.h"
#include "obd_agree_shanghai.h"
#include "obd_agree_yunjing.h"
#include "DateTime.h"
#include "upload.h"
#include "../obd/json_list.h"
#include "storage_pack.h"
#include "../obd/msg_relay.h"
#include "carinfo.h"
//#include "MySql.h"
#include "sql.h"
#include "../obd/thread_vin.h"
#include <pthread.h>
#include "data_base.h"

#include "DateTime.h"
#include <stdio.h>
#include "storage_pack.h"

static struct vin_item vin_item_list[10240];
static const uint16_t vin_item_list_size = sizeof(vin_item_list)/sizeof(vin_item_list[0]);
static uint16_t vin_item_list_index = 0;
static struct vin_item vin_item_request[1024];
static const uint16_t vin_item_request_size = sizeof(vin_item_request)/sizeof(vin_item_request[0]);
static uint16_t vin_item_request_index = 0;
static uint16_t vin_item_request_index_read = sizeof(vin_item_request)/sizeof(vin_item_request[0]);

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
        //pr_debug("data bad\n"); fflush(stdout);
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
static int fops_vin_list_load(const char *path)
{
    uint16_t index;
    const struct vin_item* _list;
    printf("[%s-%d] path:%s\n", __func__, __LINE__, path);
    memset(vin_item_list, 0, sizeof(vin_item_list));
    vin_item_list_index = 0;
    _vin_list_load(path, vin_item_list, vin_item_list_size);
    for(index=0; index<vin_item_list_size; index++)
    {
        _list = &vin_item_list[index];
        if(strlen(_list->vin)<=0) continue;
        printf("vin_item_list[%d]:\t%s \t\t%s\n", index, _list->sn, _list->vin);  fflush(stdout);
    }
    memset(&vin_item_request, 0, sizeof(vin_item_request));
    memcpy(vin_item_request[0].sn, "440303ZA0CK90N0224", 18);
    memcpy(vin_item_request[1].sn, "440303ZA0CK90N0210", 18);
    memcpy(vin_item_request[2].sn, "440303ZA0CK90N0220", 18);
    //memcpy(vin_item_request[3].sn, "440303ZA0CK90N0230", 18);
    //memcpy(vin_item_request[4].sn, "440303ZA0CK90N0240", 18);
    //memcpy(vin_item_request[5].sn, "440303ZA0CK90N0250", 18);
    //memcpy(vin_item_request[6].sn, "440303ZA0CK90N0260", 18);
    //memcpy(vin_item_request[7].sn, "440303ZA0CK90N0270", 18);
    return 0;
}
static int fops_vin_list_search(const char sn[], char vin[])
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
static int fops_vin_list_insert(const char* const sn, const char* const vin)
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
//static void __thread_vin_request_add(const char* const sn)
//{
//    struct vin_item *_item;
//    //pthread_mutex_lock (&vin_lock);
//    _item = &vin_item_request[vin_item_request_index++];
//    vin_item_request_index = vin_item_request_index%vin_item_request_size;
//    memset(_item, 0, sizeof(struct vin_item));
//    memcpy(_item->sn, sn, strlen(sn));
//    //pthread_mutex_unlock (&vin_lock);
//}
static void fops_request_add(const char* const sn)
{
    struct vin_item *_item;
    //pthread_mutex_lock (&vin_lock);
    _item = &vin_item_request[vin_item_request_index++];
    vin_item_request_index = vin_item_request_index%vin_item_request_size;
    memset(_item, 0, sizeof(struct vin_item));
    memcpy(_item->sn, sn, strlen(sn));
}
static void fops_request_del(const char* const sn)
{
    uint16_t index;
    struct vin_item *_item;
    //pthread_mutex_lock (&vin_lock);
    //printf("[%s-%d] sn:%s\n", __func__, __LINE__, sn);
    if('\0'==sn[0]) return;
    for(index=0; index<vin_item_request_size; index++)
    {
        _item = &vin_item_request[index];
        if(0==strcmp(sn, _item->sn))
        {
            //printf("[%s-%d] sn:%s\n", __func__, __LINE__, sn);
            memset(_item, 0, sizeof(struct vin_item));
        }
    }
    //pthread_mutex_unlock (&vin_lock);
}
static int fops_request_get(char _sn[])
{
    uint16_t index;
    struct vin_item *_item;
    //pthread_mutex_lock (&vin_lock);
    for(index=vin_item_request_index_read+1; index<vin_item_request_size; index++)
    {
        _item = &vin_item_request[index];
        if(strlen(_item->sn)>=8)
        {
            memcpy(_sn, _item->sn, strlen(_item->sn));
            //pthread_mutex_unlock (&vin_lock);
            //printf("[%s-%d] vin_item_request_index_read..0: %d | %d \r\n", __func__, __LINE__, vin_item_request_index_read, index);
            vin_item_request_index_read = index;
            return 0;
        }
    }
    for(index=0; index<vin_item_request_index_read; index++)
    {
        _item = &vin_item_request[index];
        if((strlen(_item->sn)>=8) && (index!=vin_item_request_index_read))
        {
            memcpy(_sn, _item->sn, strlen(_item->sn));
            //pthread_mutex_unlock (&vin_lock);
            //printf("[%s-%d] vin_item_request_index_read..1: %d | %d \r\n", __func__, __LINE__, vin_item_request_index_read, index);
            vin_item_request_index_read = index;
            return 0;
        }
    }
    //pthread_mutex_unlock (&vin_lock);
    return -1;
}

struct obd_vin_fops _obd_vin_fops = {
    .load = fops_vin_list_load,
    .search = fops_vin_list_search,
    .insert = fops_vin_list_insert,
    .req_add = fops_request_add,
    .req_del = fops_request_del,
    .req_get = fops_request_get,
};

struct obd_agree_obj* obd_agree_fops_constructed(struct obd_agree_obj* const _obd_fops, void* const _obj_fops)
{
    struct obd_agree_obj* const _obj = (struct obd_agree_obj* const)_obj_fops;
    if(NULL==_obj_fops) return NULL;
    memcpy(_obj_fops, _obd_fops, sizeof(struct obd_agree_obj));
    return _obj;
}


static int handle_encode(struct obd_agree_obj* const _obd_fops, const void * const data, const uint16_t _dsize, uint8_t pack[], const uint16_t _psize)
{
    const struct shanghai_userdef msg = {data, _dsize};
    // encode
    return _obd_fops->fops->encode_pack_general(GEN_PACK_USERDEF, _obd_fops, &msg, pack, _psize);
}
#include "DateTime.h"
void UTC2hhmmss(const uint32_t times, uint8_t buf[], const size_t _size)
{
    (void)_size;
    DateTime      utctime   = {.year = 1970, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 0};
    DateTime      localtime = {.year = 1970, .month = 1, .day = 1, .hour = 8, .minute = 0, .second = 0};
    if(times > INT32_MAX)
    {
      utctime = GregorianCalendarDateAddSecond(utctime, INT32_MAX);
      utctime = GregorianCalendarDateAddSecond(utctime, (int)(times - INT32_MAX));
    }
    else
    {
      utctime = GregorianCalendarDateAddSecond(utctime, (int)times);
    }

    GregorianCalendarDateToModifiedJulianDate(utctime);
    localtime = GregorianCalendarDateAddHour(utctime, 8);
    //gpstime   = GregorianCalendarDateAddSecond(utctime, 18);
    //gpstimews = GregorianCalendarDateToGpsWeekSecond(gpstime);

#if debug_log
    printf("Local | %d-%.2d-%.2d %.2d:%.2d:%.2d | timezone UTC+8\n",
           localtime.year, localtime.month, localtime.day,
           localtime.hour, localtime.minute, localtime.second);

    printf("UTC   | %d-%.2d-%.2d %.2d:%.2d:%.2d \n",
           utctime.year, utctime.month, utctime.day,
           utctime.hour, utctime.minute, utctime.second);
    fflush(stdout);
#endif
    //snprintf((char *)buf, (size_t)_size, "%02d%02d%02d", localtime.hour, localtime.minute, localtime.second);
    //snprintf((char *)buf, (size_t)_size, "%1d%1d%1d%1d%1d%1d", localtime.year%100, localtime.month, localtime.day, localtime.hour, localtime.minute, localtime.second);
    buf[0] = localtime.year%100;
    buf[1] = localtime.month%12;
    buf[2] = localtime.day%31;
    buf[3] = localtime.hour%24;
    buf[4] = localtime.minute%59;
    buf[5] = localtime.second%59;
}
static int login(struct obd_agree_obj* const _obd_fops, const uint32_t UTC, const uint16_t count, const uint8_t ICCID[], const uint8_t VIN[], const char *att, uint8_t buf[], const uint16_t _size)
{
    struct shanghai_login msg;
    // 该协议不需要这两个数据
    (void) VIN;
    (void) att;
    memset(&msg, 0, sizeof (msg));
    //msg.UTC[0] = UTC&0xFF;   // 先简单测试
    UTC2hhmmss(UTC, msg.UTC, sizeof (msg.UTC));
    msg.count = count;
    memcpy(msg.ICCID, ICCID, sizeof (msg.ICCID)-1);
    // encode
    return _obd_fops->fops->encode_pack_general(GEN_PACK_LOGIN, _obd_fops, &msg, buf, _size);
}

static int logout(struct obd_agree_obj* const _obd_fops, const uint32_t UTC, const uint16_t count, uint8_t buf[], const uint16_t _size)
{
    struct shanghai_logout msg;
    memset(&msg, 0, sizeof (msg));
    //msg.UTC[0] = UTC&0xFF;   // 先简单测试
    UTC2hhmmss(UTC, msg.UTC, sizeof (msg.UTC));
    msg.count = count;
    // encode
    return _obd_fops->fops->encode_pack_general(GEN_PACK_LOGOUT, _obd_fops, &msg, buf, _size);
}

static int utc(struct obd_agree_obj* const _obd_fops, uint8_t buf[], const uint16_t _size)
{
    /*
     * A4.5.4  终端校时
     *         车载终端校时的数据单元为空。
     */
    // encode
    return _obd_fops->fops->encode_pack_general(GEN_PACK_UTC, _obd_fops, NULL, buf, _size);
}

static int report(struct obd_agree_obj* const _obd_fops, const void * const msg, uint8_t buf[], const uint16_t _size)
{
    //struct shanghai_report_real *msg = report_msg(_obd, report_msg_buf, sizeof(report_msg_buf));
    // encode
    return _obd_fops->fops->encode_pack_general(GEN_PACK_REPORT_REAL, _obd_fops, msg, buf, _size);
}

static int report_later(struct obd_agree_obj* const _obd_fops, const void * const msg, uint8_t buf[], const uint16_t _size)
{
    //struct shanghai_report_real *msg = report_msg(_obd, report_msg_buf, sizeof(report_msg_buf));
    // encode
    return _obd_fops->fops->encode_pack_general(GEN_PACK_REPORT_LATER, _obd_fops, msg, buf, _size);
}

struct obd_agree_fops_base _obd_fops_base =
{
    .vin = {
        .load = fops_vin_list_load,
        .search = fops_vin_list_search,
        .insert = fops_vin_list_insert,
        .req_add = fops_request_add,
        .req_del = fops_request_del,
        .req_get = fops_request_get,
    },
    .pack =
    {
        .encode = handle_encode,
        .login = login,
        .logout = logout,
        .utc = utc,
        .report = report,
        .report_later = report_later,
    }
};

int obd_fops_decode_server(struct obd_agree_obj* const _obd_fops, const uint8_t pack[], const uint16_t _psize, void* const _msg_buf, const uint16_t _msize, \
                           struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print)
{
    int len = 0;
    char filename[128];
    //int relay=0;
#if 0
    struct sql_storage_item _items_report[64];      // 数据项
    /*struct data_base_obj _db_report_tmp = {
        .fops = &_data_base_fops,
        ._format = sql_items_format_report,
        ._format_size = _sql_items_format_report_size,
        ._items = _items_report,
        ._items_size = _sql_items_format_report_size,
        .tbl_name = "tbl_obd_4g",
        .update_flag = 0,
        .sql_query = "",
    };*/
    //struct data_base_obj* const _db_report = &_db_report_tmp;
    char _db_report_buf[sizeof(struct data_base_obj)];
    struct data_base_obj* const _db_report = db_obj_report.fops->constructed(&db_obj_report, _db_report_buf, sql_items_format_report, _sql_items_format_report_size, _items_report, _sql_items_format_report_size, "tbl_obd_4g");
#endif
#if 0
    for(len=0; len<40; len++)
    {
        printf("\033[1A"); //先回到上一行
        printf("\033[K");  //清除该行
    }
#endif
    //printf("@%s-%d \n", __func__, __LINE__);
    len = _obd_fops->fops->decode(_obd_fops, pack, _psize, _msg_buf, _msize);
    //printf("@%s-%d \n", __func__, __LINE__);
    //printf("@%s-%d decode pack len : %d \n", __func__, __LINE__, len);
    _print->fops->print(_print, "@%s-%d decode pack len : %d \n", __func__, __LINE__, len); //fflush(stdout);
    if(len<0) return -1;
    //device->type = _obd_fops->fops->protocol;
    _db_report->fops->init(_db_report); // MySqlInit();
    /*switch (_obd_fops->fops->protocol) // switch (_agree_ofp->protocol())
    {
        case PRO_TYPE_CCU:    // CCU
            //
            break;
        case PRO_TYPE_YJ:    // YunJing
            _print->fops->print(_print, "协议类型：云景OBD协议 \n"); //fflush(stdout);
            break;
        case PRO_TYPE_SHH:    // ShangHai
            _print->fops->print(_print, "协议类型：上海OBD协议 \n"); //fflush(stdout);
            //protocol_shanghai(print, &relay, _agree_ofp, (const struct general_pack_shanghai* const)_msg_buf, device, csend, _buf, _bsize);
            break;
        default:
            break;
    }*/
    //protocol_yunjing(print, &relay, _agree_ofp, (const struct general_pack_shanghai* const)_msg_buf, device, csend, _buf, _bsize);
    //printf("[@%s-%d] _print:%p fops:%p print:%p \n", __func__, __LINE__, _print, _print->fops, _print->fops->print);
    _print->fops->print(_print, "协议:[%s]\t", _obd_fops->fops->agree_des);
    _obd_fops->fops->protocol_server(_obd_fops, _ofp_data, _db_report, _print);
    //printf("@%s-%d \n", __func__, __LINE__);
    //insert_sql();
    /*ret = */_db_report->fops->insert_sql(_db_report/*, _conn_ofp*/);
    _db_report->fops->clear(_db_report);
    _db_report->fops->close(_db_report); // MySqlClose();
    // 消息转发
    //printf("relay_fd:%d relay flag : %d \n", device->relay_fd, relay); fflush(stdout);
    //if((device->relay_fd>=0) && (1==relay)) relay_msg(device->relay_fd, pack, len);
    // save log
    _print->fops->utc_format(_print, time(NULL), (uint8_t *)_obd_fops->UTC, 6);
    memset(filename, 0, sizeof (filename));
    if(strlen(_obd_fops->sn)>=12)
    {
        snprintf(filename, sizeof (filename)-1, "./log/jsons-%s.txt", _obd_fops->sn);
    }
    else if(strlen(_obd_fops->VIN)>=8)
    {
        snprintf(filename, sizeof (filename)-1, "./log/jsons-%s.txt", _obd_fops->VIN);
    }
    else
    {
        time_t timer;
        timer = time(NULL);
        _print->fops->date2filename(_print, timer, filename, sizeof(filename), "jsons-");
    }
    //int json_device_save(const char* filename, const struct device_list* const device, const uint8_t pack[], const uint16_t _psize)
    json_obd_save(filename, _obd_fops, pack, len);
    //server_log_write_to_file(_buf, strlen(_buf));
    /*if(_obd_fops->_print)
    {
        //printf("%s", _ofp_data->_print_buf);
        printf("%s", _print->__stream);
        //_print->fops->fflush(_print);
        fflush(stdout);
    }*/
    return 0;
}

int obd_fops_decode_client(struct obd_agree_obj* const _obd_fops, const uint8_t pack[], const uint16_t _psize, void* const _msg_buf, const uint16_t _msize, struct obd_agree_ofp_data* const _ofp_data, struct msg_print_obj* const _print)
{
    int len = 0;
    int ret = 0;
    struct sql_storage_item _items_report[64];      // 数据项
    struct data_base_obj _db_report = {
        .fops = &_data_base_fops,
        ._format = sql_items_format_report,
        ._format_size = _sql_items_format_report_size,
        ._items = _items_report,
        ._items_size = _sql_items_format_report_size,
        .tbl_name = "tbl_obd_4g",
        .update_flag = 0,
        .sql_query = "",
    };
    if(NULL!=_ofp_data) _ofp_data->_tlen = 0;
//    for(len=0; len<40; len++)
//    {
//        pr_debug("\033[1A"); //先回到上一行
//        pr_debug("\033[K");  //清除该行
//    }
    len = _obd_fops->fops->decode(_obd_fops, pack, _psize, _msg_buf, _msize);
    _print->fops->print(_print, "\n\n[%s-%d] decode_client len : %d \n", __func__, __LINE__, len); //fflush(stdout);
    //printf("[%s-%d] \n", __func__, __LINE__); fflush(stdout);
    //printf("\n[%s-%d] \n", __func__, __LINE__); fflush(stdout);
    if(len<0) return ERR_CLIENT_PACKS;
    /*switch (_agree_ofp->protocol) // switch (_agree_ofp->protocol())
    {
        case PRO_TYPE_CCU:    // CCU
            //
            break;
        case PRO_TYPE_YJ:     // 云景
            ret = protocol_yunjing(_agree_ofp, (const struct general_pack_shanghai*)_msg_buf, _tbuf, _tsize, _tlen);
            break;
        case PRO_TYPE_SHH:    // ShangHai
            //pr_debug("协议类型：上海OBD协议 \n"); fflush(stdout);
            ret = protocol_shanghai(_agree_ofp, (const struct general_pack_shanghai*)_msg_buf, _tbuf, _tsize, _tlen);
            break;
        default:
            ret = ERR_CLIENT_CMD;
            break;
    }*/
    _print->fops->print(_print, "协议类型:[%s]\n", _obd_fops->fops->agree_des);
    ret = _obd_fops->fops->protocol_client(_obd_fops, _ofp_data, &_db_report, _print);
    return ret;
}
