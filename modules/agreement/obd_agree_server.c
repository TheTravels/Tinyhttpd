/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : obd_agree_server.c
* Author             : Merafour
* Last Modified Date : 11/12/2019
* Description        : OBD agreement obj define.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#include "obd_agree_shanghai.h"
#include "encrypt.h"
#include "obd_agree_yunjing.h"
#include <string.h>
#include <stdio.h>
//#include <memory.h>
#include <pthread.h>
#include <malloc.h>
#include "upload.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdarg.h>

#include <time.h>
#include "agreement.h"
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
#include "obd_agree_fops.h"
#include "obd_agree_shanghai.h"

#define  debug_log     0
#ifndef debug_log
#define  debug_log     1
#endif

static const char DeviceListPath[] = "./upload/Device.list";
static const char PathPrefix[] = "./upload";

// 需要 GNU 扩展支持, -std=gnu++11
#define container_of(ptr, type, member) ({   const typeof(((type *)0)->member)*__mptr = (ptr);    (type *)((char *)__mptr - offsetof(type, member)); })

#if debug_log
//#define pr_debug(fmt, ...) printf(fmt, ##__VA_ARGS__); fflush(stdout);
int pr_debug(const char *__format, ...)
{
    static char text[1024*10];
    static char wbuffer[1024*10];
    static int windex=0; // 写指针
    static uint32_t seek=0;
    int _size=0;
    FILE* fd=NULL;
    va_list ap;
    va_start(ap, __format);
    //vprintf(__format, ap);
    memset(text, 0, sizeof (text));
    //snprintf(text, sizeof (text), __format, ap);
    vsprintf(text, __format, ap);
    va_end(ap);
    //if(print) printf(text); // fflush(stdout);
    _size = strlen (text);
    memcpy(&wbuffer[windex], text, _size);
    windex += _size;
    if(windex>1024)
    {
        if(0==seek)
        {
            fd = fopen(pr_path, "w");
            fclose(fd);
        }
        fd = fopen(pr_path, "ab");
        if(NULL!=fd)
        {
            fwrite(wbuffer, windex, 1, fd);
            fflush(fd);
            fclose(fd);
            memset(wbuffer, 0, sizeof (wbuffer));
            seek = windex;
            windex = 0;
        }
        //fflush(stdout);
    }
    return 0;
}
#else
#define pr_debug(fmt, ...) ;
#endif

/*static int msg_print(char *__stream, const size_t __n, const char *__format, ...)
{
    char* text=NULL;
    size_t _size=0;
    va_list ap;
    _size = strlen(__stream);
    if(_size>=__n) return -1;
    text = &__stream[_size];
    va_start(ap, __format);
    //vprintf(__format, ap);
    //snprintf(text, sizeof (text), __format, ap);
    vsprintf(text, __format, ap);
    va_end(ap);
    return 0;
}*/

static struct shanghai_login login_pack;
static int handle_request_login(const struct general_pack_shanghai* const _pack, struct msg_print_obj* const _print)
{
    const struct shanghai_login *const request = (const struct shanghai_login *const)(_pack->data);
    //printf("登入时间:%s\n", request->UTC);
    //printf("@%s-%d \n", __func__, __LINE__);
    _print->fops->print(_print, "登入时间:%04d.%02d.%02d %02d:%02d:%02d \t", request->UTC[0]+2000, request->UTC[1], request->UTC[2]\
            , request->UTC[3], request->UTC[4], request->UTC[5]);
    //printf("@%s-%d \n", __func__, __LINE__);
    _print->fops->print(_print, "登入流水号:%d \t", request->count);
    //printf("@%s-%d \n", __func__, __LINE__);
    _print->fops->print(_print, "SIM 卡号:%s \n", request->ICCID);
    //printf("@%s-%d \n", __func__, __LINE__);
    memcpy(&login_pack, request, sizeof (login_pack));   // save login info
    //pr_debug("Login : %s \n", request->UTC);
    // fflush(stdout);
    return 0;
}
static int handle_request_login_yj(const struct general_pack_shanghai* const _pack, struct msg_print_obj* const _print)
{
    const struct yunjing_login *const request = (const struct yunjing_login *const)(_pack->data);
    //printf("登入时间:%s\n", request->UTC);
    _print->fops->print(_print, "登入时间:%04d.%02d.%02d %02d:%02d:%02d \t", request->UTC[0]+2000, request->UTC[1], request->UTC[2]\
            , request->UTC[3], request->UTC[4], request->UTC[5]);
    _print->fops->print(_print, "登入流水号:%d \t", request->count);
    _print->fops->print(_print, "设备序列号:%s\n", request->sn);
    memcpy(&login_pack, request, sizeof (login_pack));   // save login info
    //pr_debug("Login : %s \n", request->UTC);
    // fflush(stdout);
    return 0;
}
static int handle_request_logout(const struct general_pack_shanghai* const _pack, struct msg_print_obj* const _print)
{
    const struct shanghai_logout *const msg = (const struct shanghai_logout *const)(_pack->data);
    //if(msg->count != login_pack.count) return -1;
    //pr_debug("Logout : %s \n", msg->UTC);
    _print->fops->print(_print, "登出时间:%04d.%02d.%02d %02d:%02d:%02d \t", msg->UTC[0]+2000, msg->UTC[1], msg->UTC[2]\
            , msg->UTC[3], msg->UTC[4], msg->UTC[5]);
    _print->fops->print(_print, "登出流水号:%d\n", msg->count);
    // fflush(stdout);
    return 0;
}
static int handle_request_logout_yj(const struct general_pack_shanghai* const _pack, struct msg_print_obj* const _print)
{
    const struct yunjing_logout *const msg = (const struct yunjing_logout *const)(_pack->data);
    //if(msg->count != login_pack.count) return -1;
    //pr_debug("Logout : %s \n", msg->UTC);
    _print->fops->print(_print, "登出时间:%04d.%02d.%02d %02d:%02d:%02d \t", msg->UTC[0]+2000, msg->UTC[1], msg->UTC[2]\
            , msg->UTC[3], msg->UTC[4], msg->UTC[5]);
    _print->fops->print(_print, "登出流水号:%d \t", msg->count);
    _print->fops->print(_print, "设备序列号:%s\n", msg->sn);
    // fflush(stdout);
    return 0;
}

static uint8_t obd_buf[1024*10];
static int handle_report_real(struct obd_agree_obj* const _agree_ofp, const struct general_pack_shanghai *const _pack, struct obd_agree_ofp_data* const _ofp_data, struct msg_print_obj* const _print,  struct data_base_obj* const _db_report)
{
    const struct shanghai_report_real *const msg = (const struct shanghai_report_real *const)(_pack->data);
    uint16_t index=0;
//    int32_t value_32;
//    int16_t value_16;
//    int8_t value_8;
//    uint16_t data_len=0;
    static const char* const gps_valid[2] = {"有效", "无效"};
    static const char* const gps_longitude[2] = {"东经", "西经"};
    static const char* const gps_latitude[2] = {"北纬", "南纬"};
    int i=0;
    struct report_head* nmsg=NULL;  // next msg
    const struct report_head* fault=NULL;
    const struct shanghai_data_obd *obd=NULL;
    const struct shanghai_data_stream *stream=NULL;
    const struct shanghai_data_att *att=NULL;
    char data[7][128];
    uint8_t _buffer[512];

        _print->fops->print(_print, "Report [%d]: Time: %04d.%02d.%02d %02d:%02d:%02d \n", msg->count, msg->UTC[0]+2000, msg->UTC[1], msg->UTC[2]\
                , msg->UTC[3], msg->UTC[4], msg->UTC[5]);
        //fflush(stdout);
    memset(_buffer, 0, sizeof (_buffer));
    sprintf((char*)_buffer, "%04d.%02d.%02d %02d:%02d:%02d", msg->UTC[0]+2000, msg->UTC[1], msg->UTC[2], msg->UTC[3], msg->UTC[4], msg->UTC[5]);
    _db_report->fops->insert_item_string(_db_report, "UTC", (char*)_buffer);
    // 消息，可包含多条
    nmsg = msg->msg;
    //pr_debug("msg.msg : %d \n", (NULL!=msg->msg)); // fflush(stdout);
    while(NULL!=nmsg)
    {
        //pr_debug("type_msg : %d \n", nmsg->type_msg); // fflush(stdout);
        switch (nmsg->type_msg)
        {
            // 1） OBD 信息数据格式和定义见表 A.6 所示。
            case MSG_OBD:       // 0x01  OBD 信息
                obd = (const struct shanghai_data_obd *) container_of(nmsg, struct shanghai_data_obd, head);
                // 序列号过滤信息
                /*if((filter_sn_size>0) && (0==strncmp((char*)(&obd->SVIN[6]), filter_sn, filter_sn_size)) )
                {
                    *print = 1;
                }
                if((filter_sn_size>0) && (0==strncmp((char*)(&obd->CVN[6]), filter_sn, filter_sn_size)) )
                {
                    *print = 1;
                }*/
                //if(*print)
                {
                    memset(_buffer, 0, sizeof (_buffer));
                    _print->fops->print(_print, "OBD 诊断协议: %d (“0”代表 IOS15765，“1”代表IOS27145，“2”代表 SAEJ1939，“0xFE”表示无效)\n", obd->protocol);
                    _print->fops->print(_print, "MIL 状态: %d (“0”代表未点亮，“1”代表点亮。“0xFE”表示无效)\n", obd->MIL);
                    memset(_buffer, 0, sizeof (_buffer));
                    //itoa(obd->status, _buffer, 2);
                    //printf("诊断支持状态: %d %s\n", obd->status, _buffer);
                    _print->fops->print(_print, "诊断支持状态: %d \n", obd->status);
                    memset(_buffer, 0, sizeof (_buffer));
                    //itoa(obd->ready, _buffer, 2);
                    //printf("诊断就绪状态: %d %s\n", obd->ready, _buffer);
                    _print->fops->print(_print, "诊断就绪状态: %d \n", obd->ready);
                    memset(data, 0, sizeof (data));
                    CarInfoSearch(CarListPath, (char*)obd->VIN, data[0]);
                    _print->fops->print(_print, "车辆识别码（VIN）: %s 车型：%s 车牌:%s \n", obd->VIN, data[1], data[6]);
                    _print->fops->print(_print, "软件标定识别号: %s \n", obd->SVIN);
                    if(17==strlen((char*)obd->VIN))
                    {
                        memset(_agree_ofp->VIN, 0, sizeof (_agree_ofp->VIN));
                        memcpy(_agree_ofp->VIN, obd->VIN, sizeof (obd->VIN));
                    }
                    _print->fops->print(_print, "标定验证码（CVN）: %s \n", obd->CVN);
                    if(17==strlen((char*)obd->VIN)) _db_report->fops->insert_item_string(_db_report, "vin", (char*)obd->VIN);
                    else _db_report->fops->insert_item_string(_db_report, "vin", "11112222333344445");
                    _db_report->fops->insert_item_int(_db_report, "prot", obd->protocol);
                    _db_report->fops->insert_item_int(_db_report, "mil", obd->MIL);
                    _db_report->fops->insert_item_int(_db_report, "status", obd->status);
                    _db_report->fops->insert_item_int(_db_report, "ready", obd->ready);
                    _db_report->fops->insert_item_string(_db_report, "svin", (char*)obd->SVIN);
                    _db_report->fops->insert_item_string(_db_report, "cin", (char*)obd->CVN);
                    _print->fops->print(_print, "标定验证码（CVN）: %s  match_fw=%d fw:0x%04X\n", (char*)obd->CVN, match_fw((char*)obd->CVN), _agree_ofp->fw_update);
                    //if((0!=match_fw(obd->CVN)) || ((0==match_fw(obd->CVN)) && (0x9911 != device->fw_update)))
                    if(0x9911 != _agree_ofp->fw_flag) // 记录固件信息
                    {
                        _agree_ofp->fw_flag = 0x9911;
                        sql_insert_fw_update((char*)obd->VIN, ".bin", "Old_FW", (char*)obd->CVN, (char*)&obd->SVIN[6], match_fw((char*)obd->CVN)); // 旧固件标记
                    }
                    if((0!=match_fw((char*)obd->CVN)) && (_agree_ofp->fw_update>(10/10)))  // 每一百秒推送一次更新
                    {
                        // 推送更新,发送推送更新指令
#if 0
                        struct upload* _load = NULL;
                        uint8_t buffer[sizeof(struct upload)];
                        int len = 0;
                        device->fw_update=0;
                        device->fw_flag=0;
                        _load = upload_init(UPLOAD_PUSH, 0, 10*1024, get_fw_crc(), "OBD-4G", "000000000000", 12);
                        len = upload_encode(_load, buffer, sizeof (buffer));
                        len = _obd_obj->fops->base->pack.encode(_agree_ofp, buffer, len, obd_buf, sizeof (obd_buf));
#else
                        int len = 0;
                        _agree_ofp->fw_update=0;
                        _agree_ofp->fw_flag=0;
                        len = _agree_ofp->fops->upload_push(_agree_ofp, 0, get_fw_crc()&0x0000FFFF, obd_buf, sizeof (obd_buf));
#endif
                        if(NULL != _ofp_data) //csend(device->socket, obd_buf, len); // 发送数据到客户端
                        {
                            memset(_ofp_data->_tbuf, 0, sizeof(_ofp_data->_tbuf));
                            if(len>sizeof(_ofp_data->_tbuf)) len=sizeof(_ofp_data->_tbuf);
                            memcpy(_ofp_data->_tbuf, obd_buf, len);
                            _ofp_data->_tlen = len;
                        }
                    }
                    if(_agree_ofp->fw_update<1000) _agree_ofp->fw_update++;


                    memset(_buffer, 0, sizeof (_buffer));
                    memcpy(_buffer, obd->IUPR, 36);
                    _print->fops->print(_print, "MSG_OBD IUPR: %s \n", (char*)(_buffer));
                    _print->fops->print(_print, "IUPR: ");
                    memset(_buffer, 0, sizeof (_buffer));
                    for(i=0; i<18; i++)
                    {
                        _print->fops->print(_print, " %04X",obd->IUPR[i]);
                        sprintf((char*)&_buffer[strlen((char*)_buffer)], "%04X ",obd->IUPR[i]);
                    }
                    _db_report->fops->insert_item_string(_db_report, "iupr", (char*)_buffer);
                    _print->fops->print(_print, "\n");
                    _print->fops->print(_print, "故障码总数: %d \n", obd->fault_total);
                    fault = &(obd->fault_list);
                    _print->fops->print(_print, "故障码: ");  // 故障码, BYTE（4）
                    _db_report->fops->insert_item_int(_db_report, "fault_total", obd->fault_total);
                    memset(_buffer, 0, sizeof (_buffer));
                    //while(NULL!=fault)
                    for(index=0; index<obd->fault_total; index++)
                    {
                        sprintf((char*)&_buffer[strlen((char*)_buffer)], "%04X ",fault->data);
                        _print->fops->print(_print, " 0x%02X", fault->data);  // 故障码, BYTE（4）
                        fault = fault->next;  // 下一个数据
                        if(NULL==fault) break;
                    }
                    _db_report->fops->insert_item_string(_db_report, "fault", (char*)_buffer);
                    _print->fops->print(_print, "\n"); // fflush(stdout);}
                }
                break;
            // 2）数据流信息数据格式和定义见表 A.7 所示，补充数据流信息数据格式和定义见表 A.8 所示。
            case MSG_STREAM:     // 0x02  数据流信息
                stream = (const struct shanghai_data_stream *)container_of(nmsg, struct shanghai_data_stream, head);
                //if(*print)
                {
                    _print->fops->print(_print, "车速: \t\t\t0x%04X \t\t[%4d \t%5.4f km/h] (0~250.996km/h)\n", stream->speed&0xFFFF, stream->speed, FloatConvert(stream->speed, 0, 1.0/256));
                    _print->fops->print(_print, "大气压力: \t\t0x%02X \t\t[%4d \t%5.4f kPa] (0~125kPa)\n", stream->kPa&0xFF, stream->kPa, FloatConvert(stream->kPa, 0, 0.5));
                    _print->fops->print(_print, "发动机净输出扭矩: \t0x%02X \t\t[%4d \t%4d %%] (-125~125 %% “0xFF”表示无效)\n", stream->Nm&0xFF, stream->Nm, IntConvert(stream->Nm, -125, 1));
                    _print->fops->print(_print, "摩擦扭矩: \t\t0x%02X \t\t[%4d \t%4d %%] (-125~125%%)\n", stream->Nmf&0xFF, stream->Nmf, IntConvert(stream->Nmf, -125, 1));
                    _print->fops->print(_print, "发动机转速: \t\t0x%04X \t\t[%4d \t%5.4f rpm] (0~8031.875rpm)\n", stream->rpm&0xFFFF, stream->rpm, FloatConvert(stream->rpm, 0, 0.125));
                    _print->fops->print(_print, "发动机燃料流量: \t0x%04X \t\t[%4d \t%5.4f L/h] (0~3212.75L/h)\n", stream->Lh&0xFFFF, stream->Lh, FloatConvert(stream->Lh, 0, 0.05));
                    _print->fops->print(_print, "SCR 上游 NOx : \t\t0x%04X \t\t[%4d \t%5.4f ppm] (传感器输出值:-200~3212.75ppm)\n", stream->ppm_up&0xFFFF, stream->ppm_up, FloatConvert(stream->ppm_up, -200, 0.05));
                    _print->fops->print(_print, "SCR 下游 NOx : \t\t0x%04X \t\t[%4d \t%5.4f ppm] (传感器输出值:-200~3212.75ppm)\n", stream->ppm_down&0xFFFF, stream->ppm_down, FloatConvert(stream->ppm_down, -200, 0.05));
                    _print->fops->print(_print, "反应剂余量: \t\t0x%02X \t\t[%4d \t%5.4f %%] (0~100%%)\n", stream->urea_level&0xFF, stream->urea_level, FloatConvert(stream->urea_level, 0, 0.4));
                    _print->fops->print(_print, "进气量 : \t\t0x%04X \t\t[%4d \t%5.4f ppm] (0~3212.75 kgh)\n", stream->kgh&0xFFFF, stream->kgh, FloatConvert(stream->kgh, 0, 0.05));
                    _print->fops->print(_print, "SCR 入口温度: \t\t0x%04X \t\t[%4d \t%5.4f ℃] (后处理上游排气温度:-273~1734.96875℃)\n", stream->SCR_in&0xFFFF, stream->SCR_in, FloatConvert(stream->SCR_in, -273, 0.03125));
                    _print->fops->print(_print, "SCR 出口温度: \t\t0x%04X \t\t[%4d \t%5.4f ℃] (后处理下游排气温度:-273~1734.96875℃)\n", stream->SCR_out&0xFFFF, stream->SCR_out, FloatConvert(stream->SCR_out, -273, 0.03125));
                    _print->fops->print(_print, "DPF 压差: \t\t0x%04X \t\t[%4d \t%5.4f kPa] (（或 DPF排气背压）0~6425.5 kPa)\n", stream->DPF&0xFFFF, stream->DPF, FloatConvert(stream->DPF, 0, 0.1));
                    _print->fops->print(_print, "发动机冷却液温度: \t0x%02X \t\t[%4d \t%5.4f ℃] (-40~210℃)\n", stream->coolant_temp&0xFF, stream->coolant_temp, FloatConvert(stream->coolant_temp, -40, 1.0));
                    _print->fops->print(_print, "油箱液位: \t\t0x%02X \t\t[%4d \t%5.4f %%] (0~100%%)\n", stream->tank_level&0xFF, stream->tank_level, FloatConvert(stream->tank_level, 0, 0.4));
                    _print->fops->print(_print, "定位状态: \t\t%3d \t\t[-%s-%s-%s-]\n", stream->gps_status, gps_valid[stream->gps_status&0x1], gps_latitude[(stream->gps_status>>1)&0x1], gps_longitude[(stream->gps_status>>2)&0x1]);
                    _print->fops->print(_print, "经度: \t\t\t0x%08X \t[%4d \t%5.4f°] (0~180.000000°)\n", stream->longitude&0xFFFFFFFF, stream->longitude, DIntConvert(stream->longitude, 0, 0.000001));
                    _print->fops->print(_print, "纬度: \t\t\t0x%08X \t[%4d \t%5.4f°] (0~180.000000°)\n", stream->latitude&0xFFFFFFFF, stream->latitude, DIntConvert(stream->latitude, 0, 0.000001));
                    _print->fops->print(_print, "累计里程: \t\t0x%08X \t[%4u \t%5.4f km] (精度：0.1km)\n", stream->mileages_total&0xFFFFFFFF, stream->mileages_total, DIntConvert(stream->mileages_total, 0, 0.1));
                    //fflush(stdout);
                    _db_report->fops->insert_item(_db_report, "speed", FloatConvert(stream->speed, 0, 1.0/256));
                    _db_report->fops->insert_item(_db_report, "kpa", FloatConvert(stream->kPa, 0, 0.5));
                    _db_report->fops->insert_item(_db_report, "nm", IntConvert(stream->Nm, -125, 1));
                    _db_report->fops->insert_item(_db_report, "nmf", IntConvert(stream->Nmf, -125, 1));
                    _db_report->fops->insert_item(_db_report, "rpm", FloatConvert(stream->rpm, 0, 0.125));
                    _db_report->fops->insert_item(_db_report, "Lh", FloatConvert(stream->Lh, 0, 0.05));
                    _db_report->fops->insert_item(_db_report, "ppm_up", FloatConvert(stream->ppm_up, -200, 0.05));
                    _db_report->fops->insert_item(_db_report, "ppm_down", FloatConvert(stream->ppm_down, -200, 0.05));
                    _db_report->fops->insert_item(_db_report, "urea", FloatConvert(stream->urea_level, 0, 0.4));
                    _db_report->fops->insert_item(_db_report, "kgh", FloatConvert(stream->kgh, 0, 0.05));
                    _db_report->fops->insert_item(_db_report, "SCR_in", FloatConvert(stream->SCR_in, -273, 0.03125));
                    _db_report->fops->insert_item(_db_report, "SCR_out", FloatConvert(stream->SCR_out, -273, 0.03125));
                    _db_report->fops->insert_item(_db_report, "DPF", FloatConvert(stream->DPF, 0, 0.1));
                    _db_report->fops->insert_item(_db_report, "coolant_temp", FloatConvert(stream->coolant_temp, -40, 1.0));
                    _db_report->fops->insert_item(_db_report, "tank", FloatConvert(stream->tank_level, 0, 0.4));
                    _db_report->fops->insert_item_int(_db_report, "gps_status", stream->gps_status);
                    _db_report->fops->insert_item(_db_report, "lon", DIntConvert(stream->longitude, 0, 0.000001));
                    _db_report->fops->insert_item(_db_report, "lat", DIntConvert(stream->latitude, 0, 0.000001));
                    _db_report->fops->insert_item(_db_report, "mil_total", DIntConvert(stream->mileages_total, 0, 0.1));
                }
                //MySqlSeve(_pack->VIN, DIntConvert(stream->longitude, 0, 0.000001f), DIntConvert(stream->latitude, 0, 0.000001));
                break;
            case MSG_STREAM_ATT: // 0x80  补充数据流
                att = (const struct shanghai_data_att *)container_of(nmsg, struct shanghai_data_att, head);
                //if(*print)
                {
                    _print->fops->print(_print, "发动机扭矩模式 : \t%3d \t\t(0：超速失效 1：转速控制 2：扭矩控制 3：转速/扭矩控制 9：正常)\n", att->Nm_mode);
                    _print->fops->print(_print, "油门踏板: \t\t0x%02X \t\t[%3d \t\t%5.4f %%] (0~100%%)\n", att->accelerator&0xFF, att->accelerator, FloatConvert(att->accelerator, 0, 0.4));
                    _print->fops->print(_print, "累计油耗: \t\t0x%08X \t[%3u \t%5.4f L] (0~2 105 540 607.5L)\n", att->oil_consume&0xFFFFFFFF, att->oil_consume, DIntConvert(att->oil_consume, 0, 0.5));
                    _print->fops->print(_print, "尿素箱温度: \t\t0x%02X \t\t[%3d \t\t%5.4f ℃] (-40~210℃)\n", att->urea_tank_temp&0xFF, att->urea_tank_temp, FloatConvert(att->urea_tank_temp, -40, 1.0));
                    _print->fops->print(_print, "实际尿素喷射量: \t0x%08X \t[%3u \t%5.4f ml/h] (精度：0.01 ml/h per bit)\n", att->mlh_urea_actual&0xFFFFFFFF, att->mlh_urea_actual, DIntConvert(att->mlh_urea_actual, 0, 0.01));
                    _print->fops->print(_print, "累计尿素消耗 : \t\t0x%08X \t[%3u \t%5.4f g] (精度：1 g per bit)\n", att->mlh_urea_total&0xFFFFFFFF, att->mlh_urea_total, DIntConvert(att->mlh_urea_total, 0, 1.0));
                    _print->fops->print(_print, "DPF 排气温度 : \t\t0x%04X \t\t[%3d \t\t%5.4f ℃] (-273~1734.96875℃)\n", att->exit_gas_temp&0xFFFF, att->exit_gas_temp, FloatConvert(att->exit_gas_temp, -273, 0.03125));
                    //fflush(stdout);
                    _db_report->fops->insert_item_int(_db_report, "Nm_mode", att->Nm_mode);
                    _db_report->fops->insert_item(_db_report, "accelerator", FloatConvert(att->accelerator, 0, 0.4));
                    _db_report->fops->insert_item(_db_report, "oil_consume", DIntConvert(att->oil_consume, 0, 0.5));
                    _db_report->fops->insert_item(_db_report, "urea_temp", FloatConvert(att->urea_tank_temp, -40, 1.0));
                    _db_report->fops->insert_item(_db_report, "urea_actual", DIntConvert(att->mlh_urea_actual, 0, 0.01));
                    _db_report->fops->insert_item(_db_report, "urea_total", DIntConvert(att->mlh_urea_total, 0, 1.0));
                    _db_report->fops->insert_item(_db_report, "gas_temp", FloatConvert(att->exit_gas_temp, -273, 0.03125));
                    _db_report->fops->insert_item_int(_db_report, "version", _agree_ofp->fw_crc);
                }
                break;
            case MSG_SMOKE: // 0x81  包含烟雾的数据流信息(自定义)
                {
                    struct yunjing_smoke* const smoke = container_of(nmsg, struct yunjing_smoke, head);
                    _print->fops->print(_print, "烟雾排温, 数据长度：2 btyes, 精度：1℃ /bit : %d\n", smoke->temperature);
                    _print->fops->print(_print, "OBD（烟雾故障码）,数据长度：2 btyes 精度：1/bit : %d\n", smoke->fault);
                    _print->fops->print(_print, "背压, 数据长度：2 btyes, 精度：1 kpa/bit : %d\n", smoke->kpa);
                    _print->fops->print(_print, "光吸收系数,数据长度：2 btyes,精度：0.01 m-l/bit : %d\n", smoke->m_l);
                    _print->fops->print(_print, "不透光度,数据长度：2 btyes,精度：0.1%/bit : %d\n", smoke->opacity);
                    _print->fops->print(_print, "颗粒物浓度,数据长度：2 btyes,精度：0.1mg/m3 /bit : %d\n", smoke->mg_per_m3);
                    _print->fops->print(_print, "光吸收系数超标报警,数据长度：2 btyes,精度：1 : %d\n", smoke->light_alarm);
                    _print->fops->print(_print, "背压报警,数据长度：2 btyes,精度：1 : %d\n", smoke->pressure_alarm);
                    _print->fops->print(_print, "N0x 值,数据长度：2 btyes,精度：1ppm : %d\n", smoke->ppm);
                }
                break;
            // 0x03-0x7F  预留
            // 0x81~0xFE  用户自定义
            default:
                break;
        }
        nmsg = nmsg->next;   // 下一个数据
    }
    // fflush(stdout);
    return 0;
}
#if 0
static long get_file_size(const char* filename)
{
    long _size=0;
    FILE* fd = fopen(filename, "r");
    if(NULL == fd)
    {
        printf("file %s not exist! \n", filename);  fflush(stdout);
        return -1;
    }
    else
    {
        fseek(fd, 0, SEEK_END);
        _size = ftell(fd);
        fseek(fd, 0, SEEK_SET);
        return _size;
    }
}
#endif
static char* load_file(const char* filename, long* const size)
{
    static char filebin[2*1024*1024]; // 2MB
    long _size=0;
    size_t count=0;
    FILE* fd = fopen(filename, "r");
    if(NULL == fd)
    {
        //printf("file %s not exist!..3 \n", filename);  // fflush(stdout);
        return NULL;
    }
    else
    {
        fseek(fd, 0, SEEK_END);
        _size = ftell(fd);
        fseek(fd, 0, SEEK_SET);
        pr_debug("%s@%d _size:%ld\n", __func__, __LINE__, _size);
        // read
        memset(filebin, 0xFF, sizeof (filebin));
        count = fread(filebin, (size_t)_size, 1, fd);
        if(1!=count) return NULL;
        //pr_debug("%s@%d fread _size:%d\n", __func__, __LINE__, _size);
        *size = _size;
        return filebin;
    }
}

static void upload_query(char* const vin, struct obd_agree_obj* const _agree_ofp, struct upload* const decode, const char* suffix, struct obd_agree_ofp_data* const _ofp_data, struct msg_print_obj* const _print)
{
    int len;
    char filename[128];
    uint8_t buffer[sizeof(struct upload)];
    char* fw;
    uint32_t checksum=0;
    long _size=0;
    int upload_flag=0;
    char Firmware[64];
    char Config[64];
    char Des[512];
    const char* file=NULL;
    /*if((filter_sn_size>0) && (0==strcmp((char*)(decode->data), filter_sn)))
    {
        *print = 1;
    }*/
    memcpy(_agree_ofp->sn, decode->data, decode->data_len);  // save sn
    //pr_debug("%s@%d device->sn: %s\n", __func__, __LINE__, device->sn);
    // data:sn
    upload_flag=0;
    memset(Firmware, 0, sizeof (Firmware));
    memset(Config, 0, sizeof (Config));
    if(decode->data_len<12) upload_flag=1; // 序列号至少 12位
    //else
    {
        // 序列号优先
        //if(0==json_list_search("./upload/Device.list", (const char*)(decode->data), JSON_LIST_UP)) upload_flag=1;
        if(0==json_list_search(DeviceListPath, (const char*)(decode->data), JSON_LIST_UP, Firmware, Config))
        {
            upload_flag=1;
        }
        if(0==upload_flag) // 序列号找不到再查询设备型号
        {
            memset(Firmware, 0, sizeof (Firmware));
            memset(Config, 0, sizeof (Config));
            if(0==json_list_search(DeviceListPath, (const char*)decode->Model, JSON_LIST_MODEL, Firmware, Config))
            {
                upload_flag=1;
            }
        }
    }
    _agree_ofp->fw_crc = decode->checksum;
    file=Config;
    if(0==upload_flag) // 强制更新
    {
        if(0==json_list_search(DeviceListPath, (const char*)(decode->data), JSON_LIST_COMPLE, Firmware, Config))
        {
            upload_flag=1;
        }
    }
    // 黑名单,具有最高优先级
    if(0==json_list_search(DeviceListPath, (const char*)(decode->data), JSON_LIST_HIT, Firmware, Config))
    {
        upload_flag=0;
    }
    if(1==upload_flag)
    {
        memset(filename, 0, sizeof (filename));
        if(0==strcmp(".cfg", suffix))
        {
            file=Config;
        }
        else if(0==strcmp(".bin", suffix))
        {
            file=Firmware;
        }
        else
        {
            file=Config;
            upload_flag = 0;
        }
        snprintf(filename, sizeof (filename)-1, "%s/%s/%s", PathPrefix, &suffix[1], file);
        fw = load_file(filename, &_size);  // 获取文件
        if(NULL==fw)
        {
            _print->fops->print(_print, "read file %s fail!\n", filename); // fflush(stdout);
            goto err;
        }
        // 计算和校验
        //checksum = fast_crc16(0, (unsigned char *)fw, decode->pack_index);
        checksum = fast_crc16(0, (unsigned char *)fw, _size);
    }
    _print->fops->print(_print, "%s@%d flag:%d Firmware:%s Config:%s\n", __func__, __LINE__, upload_flag, Firmware, Config);
    _print->fops->print(_print, "%s@%d file:%s serial number[%02d]: %s Model:%s\n", __func__, __LINE__, filename, decode->data_len, decode->data, decode->Model);
    _print->fops->print(_print, "%s@%d fread checksum:0x%04X decode->checksum:0x%04X\n", __func__, __LINE__, checksum, decode->checksum);
    //upload_flag=1;  // 强制更新
    memset(Des, 0, sizeof(Des));
    snprintf(Des, sizeof (Des)-1, "update file[%s] serial number[%02d]: %s Model:%s checksum:0x%04X decode->checksum:0x%04X", filename, decode->data_len, decode->data, decode->Model, checksum, decode->checksum);
    sql_insert_fw_update(vin, suffix, Des, filename, (char*)decode->data, decode->checksum);
    if((upload_flag) && (checksum != decode->checksum))  // need update
    {
        decode->pack_total = _size;
        decode->pack_index = decode->pack_total<<1;
#if 0
        len = strlen(filename)+1;
        decode->data_len = len;
        memcpy(decode->data, filename, len);  // send filename
#else
        memset(decode->data, 0, 128);
        //snprintf((char *)decode->data, 128-1, "%s%s", decode->Model, suffix);
        snprintf((char *)decode->data, 128-1, "%s", file);
        decode->data_len = strlen((const char *)decode->data)+1;
#endif
    }
    else  // not need update
    {
err:
        decode->pack_total = 0;
        decode->pack_index = 0;
        decode->data_len = 0;
    }
    decode->checksum = checksum;
    //_load = upload_init(UPLOAD_LOGIN, 0, 0, 0x12345678, "OBD1234567890ABCDEF", "Hello", 5);
    memset(buffer, 0, sizeof (buffer));
    len = upload_encode(decode, buffer, sizeof (buffer));   // user def
    _print->fops->print(_print, "%s@%d upload_encode len:%d :%s\n", __func__, __LINE__, len, buffer);
    //handle_encode(const void * const data, const uint16_t _dsize, uint8_t pack[], const uint16_t _psize)
    //len = _obd_obj->fops->base->pack.encode(_agree_ofp, buffer, len, obd_buf, (uint16_t)sizeof (obd_buf)); // encode
    len = _agree_ofp->fops->base->pack.encode(_agree_ofp, buffer, len, obd_buf, (uint16_t)sizeof (obd_buf)); // encode
    //if(NULL != csend) csend(device->socket, obd_buf, len); // 发送数据到客户端
    if(NULL != _ofp_data) //csend(device->socket, obd_buf, len); // 发送数据到客户端
    {
        //_print->fops->print(_print, "%s@%d len:%d :%s\n", __func__, __LINE__, len, obd_buf);
        memset(_ofp_data->_tbuf, 0, sizeof(_ofp_data->_tbuf));
        if(len>sizeof(_ofp_data->_tbuf)) len=sizeof(_ofp_data->_tbuf);
        memcpy(_ofp_data->_tbuf, obd_buf, len);
        _ofp_data->_tlen = len;
    }
}
//void upload_download(const struct agreement_ofp* _agree_ofp, struct upload* const decode, const char* suffix, const int client, void(*csend)(const int sockfd, const void *buf, const size_t len))
static void upload_download(struct obd_agree_obj* const _agree_ofp, struct upload* const decode, struct obd_agree_ofp_data* const _ofp_data, struct msg_print_obj* const _print)
{
    int len;
    char filename[128];
    const char *suffix=NULL;
    uint8_t buffer[sizeof(struct upload)];
    char* fw;
    uint32_t checksum=0;
    long _size=0;
    if(decode->data_len<strlen("./*.bin")+1)
    {
        _print->fops->print(_print, "decode data_len[%d] error!\n", decode->data_len); // fflush(stdout);
        //return;
        _size = 0; // 返回错误
        decode->data_len = 0;
        goto err;
    }
    _print->fops->print(_print, "%s@%d file:%s\n", __func__, __LINE__, decode->data);
    _print->fops->print(_print, "%s@%d total:%d seek:%d\n", __func__, __LINE__, decode->pack_total, decode->pack_index);
    memset(filename, 0, sizeof (filename));
    //snprintf(filename, sizeof (filename)-1, "%s/%s", PathPrefix, decode->Model, suffix);
#if 0
    memcpy(filename, decode->data, decode->data_len);
#else
    _size = (long)strlen((char*)decode->data);
    suffix = (char *)&decode->data[_size-3];
    snprintf(filename, sizeof (filename)-1, "%s/%s/%s", PathPrefix, suffix, decode->data);
#endif
    fw = load_file(filename, &_size);
    if(NULL==fw)
    {
        _print->fops->print(_print, "read file %s fail!\n", filename); // fflush(stdout);
        _size = 0; // 返回错误
        decode->data_len = 0;
        //return;
        goto err;
    }
    checksum = fast_crc16(0, (unsigned char *)fw, _size);
    // download时 decode.checksum存储请求下载的数据长度
    if((decode->pack_total<decode->pack_index) || (_size<(long)decode->pack_index) || (NULL==fw))
    {
        decode->data_len = 0;
    }
    else
    {
        decode->data_len = decode->down_len;
        memcpy(decode->data, &fw[decode->pack_index], decode->data_len);
    }
err:
    decode->checksum = checksum;
    memset(buffer, 0, sizeof (buffer));
    len = upload_encode(decode, buffer, sizeof (buffer));   // user def
    len = _agree_ofp->fops->base->pack.encode(_agree_ofp, buffer, len, obd_buf, sizeof (obd_buf)); // encode
    //if(NULL != csend) csend(device->socket, obd_buf, len); // 发送数据到客户端
    if(NULL != _ofp_data) //csend(device->socket, obd_buf, len); // 发送数据到客户端
    {
        memset(_ofp_data->_tbuf, 0, sizeof(_ofp_data->_tbuf));
        if(len>sizeof(_ofp_data->_tbuf)) len=sizeof(_ofp_data->_tbuf);
        memcpy(_ofp_data->_tbuf, obd_buf, len);
        _ofp_data->_tlen = len;
    }
}

static int handle_request_userdef(struct obd_agree_obj* const _agree_ofp, const struct general_pack_shanghai* const _pack, struct obd_agree_ofp_data* const _ofp_data, struct msg_print_obj* const _print)
{
    const struct shanghai_userdef *const msg = (const struct shanghai_userdef *const)(_pack->data);
    struct upload decode;
    int len;
    memset(&decode, 0, sizeof(struct upload));
    len = upload_decode(&decode, msg->data, msg->_dsize);
    if(len<0)
    {
        _print->fops->print(_print, "\nuserdef decode error!\n"); // fflush(stdout);
    }
    else
    {
#if 0
        _print->fops->print(_print, "decode cmd : 0x%02X\n", decode.cmd); // fflush(stdout);
        _print->fops->print(_print, "decode total : %d\n", decode.pack_total); // fflush(stdout);
        _print->fops->print(_print, "decode index : %d\n", decode.pack_index); // fflush(stdout);
        _print->fops->print(_print, "decode checksum : 0x%02X\n", decode.checksum); // fflush(stdout);
        _print->fops->print(_print, "decode Model : %s\n", decode.Model); // fflush(stdout);
        _print->fops->print(_print, "decode len : %d\n", decode.data_len); // fflush(stdout);
        _print->fops->print(_print, "decode data : %s\n", decode.data); // fflush(stdout);
        _print->fops->print(_print, "decode CRC : 0x%02X\n", decode.CRC); // fflush(stdout);
#endif
        switch (decode.cmd)
        {
            case UPLOAD_LOGIN:  // 登录
                //_print->fops->print(_print, "userdef Login\n"); // fflush(stdout);
                _print->fops->print(_print, "登录\n");
                break;
            case UPLOAD_VIN:
                _print->fops->print(_print, "UPLOAD_VIN\n");
                {
                    //static const char vin_path[] = "./upload/vin.list";
                    char vin[32];
                    uint8_t buffer[sizeof(struct upload)];
                    memset(vin, 0, sizeof (vin));
                    decode.pack_total = 0;
                    decode.pack_index = 0;
                    decode.data_len = 0;
                    decode.checksum = 0;
                    _print->fops->print(_print, "SN:%s\n", decode.data);
                    //if(0==create_vin_search(vin_path, (char*)decode.data, vin))
                    if(0==_agree_ofp->fops->base->vin.search((char*)decode.data, vin))
                    {
                        _print->fops->print(_print, "数据获取成功\n");
                        decode.data_len = strlen(vin);
                        memcpy(decode.data, vin, decode.data_len);
                    }
                    _print->fops->print(_print, "VIN:%s VIN:%s\n", decode.data, vin);
                    memset(buffer, 0, sizeof (buffer));
                    len = upload_encode(&decode, buffer, sizeof (buffer));   // user def
                    len = _agree_ofp->fops->base->pack.encode(_agree_ofp, buffer, len, obd_buf, sizeof (obd_buf)); // encode
                    //if(NULL != csend) csend(device->socket, obd_buf, len); // 发送数据到客户端
                    if(NULL != _ofp_data) //csend(device->socket, obd_buf, len); // 发送数据到客户端
                    {
                        memset(_ofp_data->_tbuf, 0, sizeof(_ofp_data->_tbuf));
                        if(len>sizeof(_ofp_data->_tbuf)) len=sizeof(_ofp_data->_tbuf);
                        memcpy(_ofp_data->_tbuf, obd_buf, len);
                        _ofp_data->_tlen = len;
                    }
                }
                break;
            case UPLOAD_QUERY_CFG:    // 查询
                _print->fops->print(_print, "查询 CFG\n");
                //_print->fops->print(_print, "userdef Config [cfg]\n"); // fflush(stdout);
                upload_query((char*)_pack->VIN, _agree_ofp, &decode, ".cfg", _ofp_data, _print);
                break;
            case UPLOAD_DOWNLOAD:   // 下载
                _print->fops->print(_print, "下载\n");
                //_print->fops->print(_print, "userdef DownLoad\n"); // fflush(stdout);
                upload_download(_agree_ofp, &decode, _ofp_data, _print);
                break;
            case UPLOAD_QUERY_FW:    // 查询
                _print->fops->print(_print, "查询 FW\n");
                //_print->fops->print(_print, "userdef Firmware [bin]\n"); // fflush(stdout);
                upload_query((char*)_pack->VIN, _agree_ofp, &decode, ".bin", _ofp_data, _print);
                break;
            case UPLOAD_GET_TIME:        // 校时
                _print->fops->print(_print, "校时\n");
                //_print->fops->print(_print, "userdef Get Time\n"); // fflush(stdout);
                {
                    uint8_t buffer[sizeof(struct upload)];
                    decode.cmd = UPLOAD_GET_TIME;
                    decode.pack_total = (uint32_t)time(NULL);  // UTC time
                    decode.data_len = 0;
                    memset(buffer, 0, sizeof (buffer));
                    len = upload_encode(&decode, buffer, sizeof (buffer));   // user def
                    len = _agree_ofp->fops->base->pack.encode(_agree_ofp, buffer, len, obd_buf, sizeof (obd_buf)); // encode
                    //if(NULL != csend) csend(device->socket, obd_buf, len); // 发送数据到客户端
                    if(NULL != _ofp_data) //csend(device->socket, obd_buf, len); // 发送数据到客户端
                    {
                        memset(_ofp_data->_tbuf, 0, sizeof(_ofp_data->_tbuf));
                        if(len>sizeof(_ofp_data->_tbuf)) len=sizeof(_ofp_data->_tbuf);
                        memcpy(_ofp_data->_tbuf, obd_buf, len);
                        _ofp_data->_tlen = len;
                    }
                }
                break;
            default:
                break;
        }
    }
    return 0;
}

static void upload_query_yunjing(const char* const vin, struct obd_agree_obj* const _agree_ofp, struct yunjing_userdef* const _udef, const char _sn[], const uint32_t _dev_checksum, const char* suffix, struct obd_agree_ofp_data* const _ofp_data, struct msg_print_obj* const _print)
{
    int len;
    char filename[128];
    uint8_t buffer[sizeof(struct upload)];
    char* fw;
    uint32_t checksum=0;
    long _size=0;
    const int _sn_len = strlen(_sn);
    int upload_flag=0;
    char Firmware[64];
    char Config[64];
    char Des[512];
    const char* file=NULL;
    //struct userdef_yj_qure_ack* const _qure_ack = (struct userdef_yj_qure_ack*)_udef->msg;
    struct userdef_yj_qure_ack _qure_ack;
    /*if((filter_sn_size>0) && (0==strcmp(_sn, filter_sn)))
    {
        *print = 1;
    }*/
    memcpy(_agree_ofp->sn, _sn, _sn_len);  // save sn
    //pr_debug("%s@%d device->sn: %s\n", __func__, __LINE__, device->sn);
    // data:sn
    upload_flag=0;
    memset(Firmware, 0, sizeof (Firmware));
    memset(Config, 0, sizeof (Config));
    memset(&_qure_ack, 0, sizeof (struct userdef_yj_qure_ack));
    if(_sn_len<12) upload_flag=1; // 序列号至少 12位
    //else
    {
        // 序列号优先
        //if(0==json_list_search("./upload/Device.list", (const char*)(decode->data), JSON_LIST_UP)) upload_flag=1;
        if(0==json_list_search(DeviceListPath, _sn, JSON_LIST_UP, Firmware, Config))
        {
            upload_flag=1;
        }
        if(0==upload_flag) // 序列号找不到再查询设备型号
        {
            memset(Firmware, 0, sizeof (Firmware));
            memset(Config, 0, sizeof (Config));
            //if(0==json_list_search(DeviceListPath, (const char*)decode->Model, JSON_LIST_MODEL, Firmware, Config))
            if(0==json_list_search(DeviceListPath, "OBDII-4G-YJ", JSON_LIST_MODEL, Firmware, Config))
            {
                upload_flag=1;
            }
        }
    }
    _agree_ofp->fw_crc = _dev_checksum;
    file=Config;
    if(0==upload_flag) // 强制更新
    {
        if(0==json_list_search(DeviceListPath, _sn, JSON_LIST_COMPLE, Firmware, Config))
        {
            upload_flag=1;
        }
    }
    // 黑名单,具有最高优先级
    if(0==json_list_search(DeviceListPath, _sn, JSON_LIST_HIT, Firmware, Config))
    {
        upload_flag=0;
    }
    if(1==upload_flag)
    {
        memset(filename, 0, sizeof (filename));
        if(0==strcmp(".cfg", suffix))
        {
            file=Config;
        }
        else if(0==strcmp(".bin", suffix))
        {
            file=Firmware;
        }
        else
        {
            file=Config;
            upload_flag = 0;
        }
        snprintf(filename, sizeof (filename)-1, "%s/%s/%s", PathPrefix, &suffix[1], file);
        fw = load_file(filename, &_size);  // 获取文件
        if(NULL==fw)
        {
            _print->fops->print(_print, "%s@%d read file %s fail!\n", __func__, __LINE__, filename); // fflush(stdout);
            goto err;
        }
        // 计算和校验
        //checksum = fast_crc16(0, (unsigned char *)fw, decode->pack_index);
        checksum = fast_crc16(0, (unsigned char *)fw, _size);
    }
    _print->fops->print(_print, "%s@%d flag:%d Firmware:%s Config:%s\n", __func__, __LINE__, upload_flag, Firmware, Config);
    _print->fops->print(_print, "%s@%d file:%s serial number[%02d]: %s\n", __func__, __LINE__, filename, _sn_len, _sn);
    _print->fops->print(_print, "%s@%d fread checksum:0x%04X decode->checksum:0x%04X\n", __func__, __LINE__, checksum, _dev_checksum);
    //upload_flag=1;  // 强制更新
    memset(Des, 0, sizeof(Des));
    //_print->fops->print(_print, "%s@%d Run...\n", __func__, __LINE__);
    snprintf(Des, sizeof (Des)-1, "update file[%s] serial number[%02d]: %s checksum:0x%04X decode->checksum:0x%04X", filename, _sn_len, _sn, checksum, _dev_checksum);
    sql_insert_fw_update(vin, suffix, Des, filename, _sn, _dev_checksum);
    if((upload_flag) && (checksum != _dev_checksum))  // need update
    {
        _qure_ack._total = _size;
        snprintf(_qure_ack.key, sizeof(_qure_ack.key)-1, "%s", file);
    }
    else  // not need update
    {
err:
        _qure_ack._total = 0;
    }
    _qure_ack.checksum = checksum;
    //_load = upload_init(UPLOAD_LOGIN, 0, 0, 0x12345678, "OBD1234567890ABCDEF", "Hello", 5);
    memset(buffer, 0, sizeof (buffer));
    //len = upload_encode(decode, buffer, sizeof (buffer));   // user def
    memcpy(_udef->msg, &_qure_ack, sizeof(struct userdef_yj_qure_ack));
    //len = userdef_encode_yj(_udef, buffer, sizeof (buffer));   // user def
    len = _agree_ofp->fops->userdef_encode(_agree_ofp, _udef, buffer, sizeof (buffer));
    len = _agree_ofp->fops->base->pack.encode(_agree_ofp, buffer, len, obd_buf, sizeof (obd_buf)); // encode
    //if(NULL != csend) csend(device->socket, obd_buf, len); // 发送数据到客户端
    if(NULL != _ofp_data) //csend(device->socket, obd_buf, len); // 发送数据到客户端
    {
        //_print->fops->print(_print, "%s@%d len:%d :%s\n", __func__, __LINE__, len, obd_buf);
        memset(_ofp_data->_tbuf, 0, sizeof(_ofp_data->_tbuf));
        if(len>sizeof(_ofp_data->_tbuf)) len=sizeof(_ofp_data->_tbuf);
        memcpy(_ofp_data->_tbuf, obd_buf, len);
        _ofp_data->_tlen = len;
    }
}
static void upload_query_block_yunjing(const char* const vin, struct obd_agree_obj* const _agree_ofp, struct yunjing_userdef* const _udef, const char _sn[], const uint32_t _dev_checksum, const char* suffix, struct obd_agree_ofp_data* const _ofp_data, struct msg_print_obj* const _print)
{
    int len;
    char filename[128];
    uint8_t buffer[sizeof(struct upload)];
    char* fw;
    uint32_t checksum=0;
    long _size=0;
    const int _sn_len = strlen(_sn);
    int upload_flag=0;
    char Firmware[64];
    char Config[64];
    char Des[512];
    const char* file=NULL;
    //struct userdef_yj_qure_ack* const _qure_ack = (struct userdef_yj_qure_ack*)_udef->msg;
    struct userdef_yj_qureb_ack _qureb_ack;
    /*if((filter_sn_size>0) && (0==strcmp(_sn, filter_sn)))
    {
        *print = 1;
    }*/
    memcpy(_agree_ofp->sn, _sn, _sn_len);  // save sn
    //pr_debug("%s@%d device->sn: %s\n", __func__, __LINE__, device->sn);
    // data:sn
    upload_flag=0;
    memset(Firmware, 0, sizeof (Firmware));
    memset(Config, 0, sizeof (Config));
    memset(&_qureb_ack, 0, sizeof (struct userdef_yj_qureb_ack));
    //_qureb_ack.block = 2048; // 2K
    _qureb_ack.value = 0xFF; // 填充值
    _qureb_ack.block = 2;    // 块大小 2K
    if(_sn_len<12) upload_flag=1; // 序列号至少 12位
    //else
    {
        // 序列号优先
        //if(0==json_list_search("./upload/Device.list", (const char*)(decode->data), JSON_LIST_UP)) upload_flag=1;
        if(0==json_list_search(DeviceListPath, _sn, JSON_LIST_UP, Firmware, Config))
        {
            upload_flag=1;
        }
        if(0==upload_flag) // 序列号找不到再查询设备型号
        {
            memset(Firmware, 0, sizeof (Firmware));
            memset(Config, 0, sizeof (Config));
            //if(0==json_list_search(DeviceListPath, (const char*)decode->Model, JSON_LIST_MODEL, Firmware, Config))
            if(0==json_list_search(DeviceListPath, "OBDII-4G-YJ", JSON_LIST_MODEL, Firmware, Config))
            {
                upload_flag=1;
            }
        }
    }
    _agree_ofp->fw_crc = _dev_checksum;
    file=Config;
    if(0==upload_flag) // 强制更新
    {
        if(0==json_list_search(DeviceListPath, _sn, JSON_LIST_COMPLE, Firmware, Config))
        {
            upload_flag=1;
        }
    }
    // 黑名单,具有最高优先级
    if(0==json_list_search(DeviceListPath, _sn, JSON_LIST_HIT, Firmware, Config))
    {
        upload_flag=0;
    }
    if(1==upload_flag)
    {
        long _msize=0;
        int i, j, down;
        uint16_t _block;
        memset(filename, 0, sizeof (filename));
        if(0==strcmp(".cfg", suffix))
        {
            file=Config;
        }
        else if(0==strcmp(".bin", suffix))
        {
            file=Firmware;
        }
        else
        {
            file=Config;
            upload_flag = 0;
        }
        snprintf(filename, sizeof (filename)-1, "%s/%s/%s", PathPrefix, &suffix[1], file);
        fw = load_file(filename, &_size);  // 获取文件
        if(NULL==fw)
        {
            _print->fops->print(_print, "%s@%d read file %s fail!\n", __func__, __LINE__, filename);
            goto err;
        }
        // 计算和校验
        //checksum = fast_crc16(0, (unsigned char *)fw, decode->pack_index);
        checksum = fast_crc16(0, (unsigned char *)fw, _size);
        // map
        i=0;
        _block = _qureb_ack.block*1024;
        for(_msize=0; _msize<_size; _msize+=_block)
        {
            char _byte;
            down = 0;
            for(j=0; j<_block; j++)
            {
                _byte = fw[_msize+j];
                if(0xFF!=(_byte&0xFF)) // 有数据
                {
                    down = 1;
                    _print->fops->print(_print, "%s@%d download: _seek[%08X | %02X]\n", __func__, __LINE__, _msize+j, (_byte&0xFF)); // fflush(stdout);
                    break;
                }
            }
            // 不需要下载
            if(0==down) _qureb_ack.map[i>>3] |= (0x1<<(i&0x07));
            i++;
        }
    }
    _print->fops->print(_print, "%s@%d flag:%d Firmware:%s Config:%s\n", __func__, __LINE__, upload_flag, Firmware, Config);
    _print->fops->print(_print, "%s@%d file:%s serial number[%02d]: %s\n", __func__, __LINE__, filename, _sn_len, _sn);
    _print->fops->print(_print, "%s@%d fread checksum:0x%04X decode->checksum:0x%04X\n", __func__, __LINE__, checksum, _dev_checksum);
    //upload_flag=1;  // 强制更新
    memset(Des, 0, sizeof(Des));
    //LogPrint(*print, "%s@%d Run...\n", __func__, __LINE__);
    snprintf(Des, sizeof (Des)-1, "update file[%s] serial number[%02d]: %s checksum:0x%04X decode->checksum:0x%04X", filename, _sn_len, _sn, checksum, _dev_checksum);
    sql_insert_fw_update(vin, suffix, Des, filename, _sn, _dev_checksum);
    if((upload_flag) && (checksum != _dev_checksum))  // need update
    {
        _qureb_ack._total = _size;
        snprintf(_qureb_ack.key, sizeof(_qureb_ack.key)-1, "%s", file);
    }
    else  // not need update
    {
err:
        _qureb_ack._total = 0;
    }
    _qureb_ack.checksum = checksum;
    //_load = upload_init(UPLOAD_LOGIN, 0, 0, 0x12345678, "OBD1234567890ABCDEF", "Hello", 5);
    memset(buffer, 0, sizeof (buffer));
    //len = upload_encode(decode, buffer, sizeof (buffer));   // user def
    memcpy(_udef->msg, &_qureb_ack, sizeof(struct userdef_yj_qureb_ack));
    //len = userdef_encode_yj(_udef, buffer, sizeof (buffer));   // user def
    len = _agree_ofp->fops->userdef_encode(_agree_ofp, _udef, buffer, sizeof (buffer));
    len = _agree_ofp->fops->base->pack.encode(_agree_ofp, buffer, len, obd_buf, sizeof (obd_buf)); // encode
    //if(NULL != csend) csend(device->socket, obd_buf, len); // 发送数据到客户端
    if(NULL != _ofp_data) //csend(device->socket, obd_buf, len); // 发送数据到客户端
    {
        //_print->fops->print(_print, "%s@%d len:%d :%s\n", __func__, __LINE__, len, obd_buf);
        memset(_ofp_data->_tbuf, 0, sizeof(_ofp_data->_tbuf));
        if(len>sizeof(_ofp_data->_tbuf)) len=sizeof(_ofp_data->_tbuf);
        memcpy(_ofp_data->_tbuf, obd_buf, len);
        _ofp_data->_tlen = len;
    }
}
static void upload_download_yunjing(struct obd_agree_obj* const _agree_ofp, struct yunjing_userdef* const _udef, const char _filename[], const uint32_t _seek, const uint32_t _total, const uint16_t block, struct obd_agree_ofp_data* const _ofp_data, struct msg_print_obj* const _print)
{
    int len;
    char filename[128];
    //const char *suffix=NULL;
    char suffix[4] = "bin";
    uint8_t buffer[sizeof(struct yunjing_userdef)];
    char* fw;
    //uint32_t checksum=0;
    long _size=0;
    const size_t _filename_len = strlen(_filename);
    //struct userdef_yj_ack_download* const _ack_download = (struct userdef_yj_ack_download*)_udef->msg;
    struct userdef_yj_ack_download _ack_download;
    memset(&_ack_download, 0, sizeof (struct userdef_yj_ack_download));
    _ack_download._seek = _seek;
    if(_filename_len<strlen("./*.bin")+1)
    {
        _print->fops->print(_print, "decode data_len[%d] error!\n", _filename_len); // fflush(stdout);
        //return;
        _size = 0; // 返回错误
        _ack_download.block = 0;
        _ack_download._total = 0;
        goto err;
    }
    _print->fops->print(_print, "%s@%d file:%s\n", __func__, __LINE__, _filename);
    _print->fops->print(_print, "%s@%d total:%d seek:%d block:%d\n", __func__, __LINE__, _total, _seek, block);
    _print->fops->print(_print, "%s@%d total:%08X seek:%08X block:%08X\n", __func__, __LINE__, _total, _seek, block);
    memset(filename, 0, sizeof (filename));
    //snprintf(filename, sizeof (filename)-1, "%s/%s", PathPrefix, decode->Model, suffix);
#if 0
    memcpy(filename, decode->data, decode->data_len);
#else
    _size = (long)_filename_len;
    //suffix = &filename[_size-3];
    memcpy(suffix, &_filename[_size-3], 3);
    snprintf(filename, sizeof (filename)-1, "%s/%s/%s", PathPrefix, suffix, _filename);
#endif
    fw = load_file(filename, &_size);
    if(NULL==fw)
    {
        _print->fops->print(_print, "read file %s fail!\n", filename); // fflush(stdout);
        _size = 0; // 返回错误
        _ack_download.block = 0;
        _ack_download._total = 0;
        //return;
        goto err;
    }
    //checksum = fast_crc16(0, (unsigned char *)fw, _size);
    // download时 decode.checksum存储请求下载的数据长度
    if((_total<_seek) || (_size<(long)_seek) || (NULL==fw))
    {
        _ack_download.block = 0;
        _ack_download._total = 0;
    }
    else
    {
        _ack_download.block = block;
        _ack_download._total = _total;
        memcpy(_ack_download.data, &fw[_seek], block);
    }
err:
    //decode->checksum = checksum;
    memset(buffer, 0, sizeof (buffer));
    //len = upload_encode(decode, buffer, sizeof (buffer));   // user def
    memcpy(_udef->msg, &_ack_download, sizeof(struct userdef_yj_ack_download));
    //len = userdef_encode_yj(_udef, buffer, sizeof (buffer));   // user def
    len = _agree_ofp->fops->userdef_encode(_agree_ofp, _udef, buffer, sizeof (buffer));
    len = _agree_ofp->fops->base->pack.encode(_agree_ofp, buffer, len, obd_buf, sizeof (obd_buf)); // encode
    //if(NULL != csend) csend(device->socket, obd_buf, len); // 发送数据到客户端
    if(NULL != _ofp_data) //csend(device->socket, obd_buf, len); // 发送数据到客户端
    {
        //_print->fops->print(_print, "%s@%d len:%d :%s\n", __func__, __LINE__, len, obd_buf);
        memset(_ofp_data->_tbuf, 0, sizeof(_ofp_data->_tbuf));
        if(len>sizeof(_ofp_data->_tbuf)) len=sizeof(_ofp_data->_tbuf);
        memcpy(_ofp_data->_tbuf, obd_buf, len);
        _ofp_data->_tlen = len;
    }
}
static int handle_request_userdef_yunjing(struct obd_agree_obj* const _agree_ofp, const struct general_pack_shanghai* const _pack, struct obd_agree_ofp_data* const _ofp_data, struct msg_print_obj* const _print)
{
    const struct shanghai_userdef *const msg = (const struct shanghai_userdef *const)(_pack->data);
    struct yunjing_userdef _userdef;
    int len;
    const struct userdef_yj_qure* const _qure = (const struct userdef_yj_qure*)_userdef.msg;
    memset(&_userdef, 0, sizeof(struct yunjing_userdef));
    //len = userdef_decode_yj(&_userdef, msg->data, msg->_dsize);
    len = _agree_ofp->fops->userdef_decode(_agree_ofp, &_userdef, msg->data, msg->_dsize);
    //_print->fops->print(_print, "userdef_decode_yj:%d\n", len);
    if(len<0)
    {
        _print->fops->print(_print, "userdef decode error!\n");
    }
    else
    {
#if 0
        _print->fops->print(_print, "decode cmd : 0x%02X\n", decode.cmd); // fflush(stdout);
        _print->fops->print(_print, "decode total : %d\n", decode.pack_total); // fflush(stdout);
        _print->fops->print(_print, "decode index : %d\n", decode.pack_index); // fflush(stdout);
        _print->fops->print(_print, "decode checksum : 0x%02X\n", decode.checksum); // fflush(stdout);
        _print->fops->print(_print, "decode Model : %s\n", decode.Model); // fflush(stdout);
        _print->fops->print(_print, "decode len : %d\n", decode.data_len); // fflush(stdout);
        _print->fops->print(_print, "decode data : %s\n", decode.data); // fflush(stdout);
        _print->fops->print(_print, "decode CRC : 0x%02X\n", decode.CRC); // fflush(stdout);
#endif
        //_print->fops->print(_print, "_userdef.type_msg:%d\n", _userdef.type_msg);
        //_print->fops->print(_print, "_userdef.count:%d\n", _userdef.count);
        switch (_userdef.type_msg)
        {
            case USERDEF_YJ_QUERY_VIN:   // 请求 VIN 码
                _print->fops->print(_print, "QUERY_VIN\n");
                //memcpy(&buffer[index], _udef->msg, 18); index += 18; // 18位SN号
                {
                    //static const char vin_path[] = "./upload/vin.list";
                    char vin[32];
                    uint8_t buffer[sizeof(struct yunjing_userdef)];
                    _userdef.type_msg = USERDEF_YJ_ACK_VIN;
                    memset(vin, 0, sizeof (vin));
                    _print->fops->print(_print, "SN:%s\n", _userdef.msg);
#if 1
                    //if(0==create_vin_search(vin_path, _userdef.msg, vin))
                    if(0==_agree_ofp->fops->base->vin.search(_userdef.msg, vin))
#else
                    thread_vin_request_add(_userdef.msg);
                    sleep(1); // 等待获取数据
                    if(0==vin_list_search(_userdef.msg, vin))
#endif
                    {
                        _print->fops->print(_print, "数据获取成功\n");
                        memset(_userdef.msg, 0, 32);
                        memcpy(_userdef.msg, vin, 17); // 17位VIN码
                    }
                    else memset(_userdef.msg, 0, 32);
                    _print->fops->print(_print, "VIN:%s VIN:%s\n", _userdef.msg, vin);
                    memset(buffer, 0, sizeof (buffer));
                    //len = userdef_encode_yj(&_userdef, buffer, sizeof (buffer));   // user def
                    len = _agree_ofp->fops->userdef_encode(_agree_ofp, &_userdef, buffer, sizeof (buffer));
                    len = _agree_ofp->fops->base->pack.encode(_agree_ofp, buffer, len, obd_buf, sizeof (obd_buf)); // encode
                    //if(NULL != csend) csend(device->socket, obd_buf, len); // 发送数据到客户端
                    if(NULL != _ofp_data) //csend(device->socket, obd_buf, len); // 发送数据到客户端
                    {
                        memset(_ofp_data->_tbuf, 0, sizeof(_ofp_data->_tbuf));
                        if(len>sizeof(_ofp_data->_tbuf)) len=sizeof(_ofp_data->_tbuf);
                        memcpy(_ofp_data->_tbuf, obd_buf, len);
                        _ofp_data->_tlen = len;
                    }
                }
                break;
            /*case USERDEF_YJ_PUSH_VIN:    // 下发 VIN 码
                memcpy(&buffer[index], _udef->msg, 17); index += 17; // 17位VIN码
                break;*/
            case USERDEF_YJ_QUERY_TIME:  // 请求校时
                _print->fops->print(_print, "校时\n");
                //_print->fops->print(_print, "userdef Get Time\n"); // fflush(stdout);
                {
                    uint8_t buffer[sizeof(struct yunjing_userdef)];
                    _userdef.type_msg = USERDEF_YJ_ACK_TIME;
                    memset(_userdef.msg, 0, 32);
                    UTC2GMT8((uint32_t)time(NULL), (uint8_t*)_userdef.msg, 6);
                    memset(buffer, 0, sizeof (buffer));
                    //len = userdef_encode_yj(&_userdef, buffer, sizeof (buffer));   // user def
                    len = _agree_ofp->fops->userdef_encode(_agree_ofp, &_userdef, buffer, sizeof (buffer));
                    len = _agree_ofp->fops->base->pack.encode(_agree_ofp, buffer, len, obd_buf, sizeof (obd_buf)); // encode
                    //if(NULL != csend) csend(device->socket, obd_buf, len); // 发送数据到客户端
                    if(NULL != _ofp_data) //csend(device->socket, obd_buf, len); // 发送数据到客户端
                    {
                        memset(_ofp_data->_tbuf, 0, sizeof(_ofp_data->_tbuf));
                        if(len>sizeof(_ofp_data->_tbuf)) len=sizeof(_ofp_data->_tbuf);
                        memcpy(_ofp_data->_tbuf, obd_buf, len);
                        _ofp_data->_tlen = len;
                    }
                }
                break;
            /*case USERDEF_YJ_PUSH_TIME:   // 下发校时
                memcpy(&buffer[index], _udef->msg, 6); index += 6; // 6位GMT+8 时间,年月日时分秒
                break;*/
            case USERDEF_YJ_DEV_FAULT:   // 设备故障
                _print->fops->print(_print, "设备故障\n");
                {
                    uint8_t count;
                    struct userdef_yj_fault* const fault = (struct userdef_yj_fault*)_userdef.msg;
                    _print->fops->print(_print, "设备故障:%d\n", fault->sum);
                    for(count=0; count<fault->sum; count++)
                    {
                        _print->fops->print(_print, "%04X\t", fault->value[count]);
                    }
                    _print->fops->print(_print, "\n");
                }
                break;
            case USERDEF_YJ_DEV_OFFLINE: // 设备离线
                // 设备离线指令数据体为空
                _print->fops->print(_print, "设备离线\n");
                break;
            case USERDEF_YJ_QUERY_CFG:   // 查询配置文件更新
                _print->fops->print(_print, "查询 CFG\n");
                _userdef.type_msg = USERDEF_YJ_ACK_CFG;
                upload_query_yunjing((char*)_pack->VIN, _agree_ofp, &_userdef, _qure->sn, _qure->checksum, ".cfg", _ofp_data, _print);
                break;
            case USERDEF_YJ_ACK_CFG:     // 响应
                _print->fops->print(_print, "ACK_CFG\n");
                break;
            case USERDEF_YJ_QUERY_FW:    // 查询固件更新
                _print->fops->print(_print, "查询 FW\n");
                _userdef.type_msg = USERDEF_YJ_ACK_FW;
                upload_query_yunjing((char*)_pack->VIN, _agree_ofp, &_userdef, _qure->sn, _qure->checksum, ".bin", _ofp_data, _print);
                break;
            case USERDEF_YJ_ACK_FW:      // 响应
                _print->fops->print(_print, "ACK_FW\n");
                break;
            case USERDEF_YJ_QUERY_FWB:    // 查询固件更新,获取固件块信息
                _print->fops->print(_print, "块查询\n");
                _userdef.type_msg = USERDEF_YJ_ACK_FWB;
                upload_query_block_yunjing((char*)_pack->VIN, _agree_ofp, &_userdef, _qure->sn, _qure->checksum, ".bin", _ofp_data, _print);
                break;
            case USERDEF_YJ_ACK_FWB:      // 响应
                _print->fops->print(_print, "ACK_FWB\n");
                break;
            case USERDEF_YJ_DOWNLOAD:     // 下载
                _print->fops->print(_print, "下载\n");
                {
                    _print->fops->print(_print, "USERDEF_YJ_DOWNLOAD\n");
                    const struct userdef_yj_download* const _download = (const struct userdef_yj_download*)_userdef.msg;
                    _userdef.type_msg = USERDEF_YJ_ACK_DOWNLOAD;
                    upload_download_yunjing(_agree_ofp, &_userdef, _download->key, _download->_seek, _download->_total, _download->block, _ofp_data, _print);
                }
                break;
            case USERDEF_YJ_ACK_DOWNLOAD: // 响应
                _print->fops->print(_print, "ACK_DOWNLOAD\n");
                break;
            case USERDEF_YJ_PUSH:         // 推送
                _print->fops->print(_print, "PUSH\n");
                break;
            default:
                _print->fops->print(_print, "DEFAULT\n");
                break;
        }
    }
    // fflush(stdout);
    return 0;
}

int obj_obd_agree_shanghai_server(struct obd_agree_obj* const _obd_fops, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print)
{
    const char* ssl_type[] = {
        "0x00: NULL",
        "0x01：数据不加密",
        "0x02：数据经过 RSA 算法加密",
        "0x03：数据经过国密 SM2 算法加密",
        "“0xFE”标识异常，“0xFF”表示无效，其他预留 ",
    };
    //const struct general_pack_shanghai* const pack = (const struct general_pack_shanghai*)_general_pack;
    const struct general_pack_shanghai* const pack = &_obd_fops->_gen_pack;
    /*if((filter_vin_size>0) && (0==strcmp((char*)(pack->VIN), filter_vin)))
    {
        _print = 1;
    }
    if((0==filter_vin_size) && (0==filter_sn_size))
    {
        _print = 1;
    }*/
    //char* const _buf = (char*)_ofp_data->_print_buf;
    //const unsigned int _bsize = sizeof(_ofp_data->_print_buf);
    _obd_fops->_print = 1;
#if 0
    _print->fops->print(_print, "起始符: %c%c \n", pack->start[0], pack->start[1]);
    _print->fops->print(_print, "命令单元: %d \n", pack->cmd);
    _print->fops->print(_print, "车辆识别号: %s\n", pack->VIN);
    _print->fops->print(_print, "软件版本号: %d \n", pack->soft_version);
    _print->fops->print(_print, "数据加密方式: %s \n", ssl_type[pack->ssl&0x3]);
    _print->fops->print(_print, "数据单元长度: %d \n", pack->data_len);
    _print->fops->print(_print, "校验码: %02X \n", pack->BCC);
#endif
    _print->fops->print(_print, "起始符:[%c%c] ", pack->start[0], pack->start[1]);
    _print->fops->print(_print, "命令:[%02X] ", pack->cmd);
    _print->fops->print(_print, "VIN:[%-17s] ", pack->VIN);
    _print->fops->print(_print, "版本号:[%02X] ", pack->soft_version);
    _print->fops->print(_print, "加密:[%-16s] ", ssl_type[pack->ssl&0x3]);
    _print->fops->print(_print, "数据长度:[%-4d] ", pack->data_len);
    _print->fops->print(_print, "BCC:[%02X] ", pack->BCC);
    if(17==strlen((char*)pack->VIN))
    {
        memset(_obd_fops->VIN, 0, sizeof (_obd_fops->VIN));
        memcpy(_obd_fops->VIN, pack->VIN, sizeof (pack->VIN));
    }
    _obd_fops->_relay = 1;
    //printf("@%s-%d \n", __func__, __LINE__);
    _print->fops->print(_print, "数据:");
    switch(pack->cmd)
    {
        case CMD_LOGIN:        // 车辆登入
            //printf("@%s-%d \n", __func__, __LINE__);
            _print->fops->print(_print, "车辆登入\n"); // fflush(stdout);
            //pack_len = handle_request_login((const struct shanghai_login *const)msg_buf, pack);
            handle_request_login(pack, _print);
            break;
        case CMD_REPORT_REAL:  // 实时信息上报
            _print->fops->print(_print, "实时信息上报\n"); // fflush(stdout);
            handle_report_real(_obd_fops, pack, _ofp_data, _print, _db_report);
            break;
        case CMD_REPORT_LATER: // 补发信息上报
            _print->fops->print(_print, "补发信息上报\n"); // fflush(stdout);
            handle_report_real(_obd_fops, pack, _ofp_data, _print, _db_report);
            break;
        case CMD_LOGOUT:       // 车辆登出
            _print->fops->print(_print, "车辆登出\n"); // fflush(stdout);
            //printf("车辆登出\n");
            //pack_len = handle_request_logout((const struct shanghai_logout *const)msg_buf, pack);
            handle_request_logout(pack, _print);
            break;
        case CMD_UTC:          // 终端校时
            _print->fops->print(_print, "终端校时\n"); // fflush(stdout);
            break;
        case CMD_USERDEF:      // 用户自定义
            _print->fops->print(_print, "自定义-"); // fflush(stdout);
            handle_request_userdef(_obd_fops, pack, _ofp_data, _print);
            _obd_fops->_relay = 0;
            break;
        default:
            _print->fops->print(_print, "default\n"); // fflush(stdout);
            _obd_fops->_relay = 0;
            break;
    }
    return 0;
}

int obj_obd_agree_yunjing_server(struct obd_agree_obj* const _obd_fops, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print)
{
    const char* ssl_type[] = {
        "0x00: NULL",
        "0x01：数据不加密",
        "0x02：数据经过 RSA 算法加密",
        "0x03：数据经过国密 SM2 算法加密",
        "“0xFE”标识异常，“0xFF”表示无效，其他预留 ",
    };
    const struct general_pack_shanghai* const pack = &_obd_fops->_gen_pack_YJ;
    _obd_fops->_print = 1;
    _print->fops->print(_print, "起始符:[%c%c] ", pack->start[0], pack->start[1]);
    _print->fops->print(_print, "命令:[%02X] ", pack->cmd);
    _print->fops->print(_print, "VIN:[%-17s] ", pack->VIN);
    _print->fops->print(_print, "版本号:[%02X] ", pack->soft_version);
    _print->fops->print(_print, "加密:[%-16s] ", ssl_type[pack->ssl&0x3]);
    _print->fops->print(_print, "数据长度:[%-4d] ", pack->data_len);
    _print->fops->print(_print, "BCC:[%02X] ", pack->BCC);
    if(17==strlen((char*)pack->VIN))
    {
        memset(_obd_fops->VIN, 0, sizeof (_obd_fops->VIN));
        memcpy(_obd_fops->VIN, pack->VIN, sizeof (pack->VIN));
    }
    _obd_fops->_relay = 1;
    _print->fops->print(_print, "数据:");
    switch(pack->cmd)
    {
        case CMD_LOGIN_YJ:        // 车辆登入
            _print->fops->print(_print, "车辆登入,云景\n"); // fflush(stdout);
            handle_request_login_yj(pack, _print);
            break;
        case CMD_REPORT_REAL_YJ:  // 实时信息上报
            _print->fops->print(_print, "实时信息上报\n"); // fflush(stdout);
            handle_report_real(_obd_fops, pack, _ofp_data, _print, _db_report);
            break;
        case CMD_REPORT_LATER_YJ: // 补发信息上报
            _print->fops->print(_print, "补发信息上报\n"); // fflush(stdout);
            handle_report_real(_obd_fops, pack, _ofp_data, _print, _db_report);
            break;
        case CMD_LOGOUT_YJ:       // 车辆登出
            _print->fops->print(_print, "车辆登出,云景\n"); // fflush(stdout);
            handle_request_logout_yj(pack, _print);
            break;
        case CMD_UTC_YJ:          // 终端校时
            _print->fops->print(_print, "终端校时\n"); // fflush(stdout);
            break;
        case CMD_USERDEF_YJ:      // 用户自定义
            _print->fops->print(_print, "自定义-"); // fflush(stdout);
            handle_request_userdef_yunjing(_obd_fops, pack, _ofp_data, _print);
            _obd_fops->_relay = 0;
            break;
        default:
            _print->fops->print(_print, "default\n"); // fflush(stdout);
            _obd_fops->_relay = 0;
            break;
    }
    return 0;
}

