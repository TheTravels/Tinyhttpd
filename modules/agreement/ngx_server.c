#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdarg.h>

#include <time.h>
#include "agreement.h"
#include "obd_agree_shanghai.h"
#include "DateTime.h"
#include "upload.h"
#include "../obd/json_list.h"
#include "storage_pack.h"
#include "../obd/msg_relay.h"
#include "carinfo.h"
//#include "MySql.h"
#include "sql.h"

#define  debug_log     0
#ifndef debug_log
#define  debug_log     1
#endif
//#undef   debug_log
#include <stdio.h>

static uint8_t obd_buf[1024*10];
static const char DeviceListPath[] = "./upload/Device.list";
static const char PathPrefix[] = "./upload";
//static char pr_path[] = "./log/debug";
//#ifdef debug_log
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

#ifdef offsetof
#undef offsetof
#endif
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:   the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({          \
    const typeof(((type *)0)->member)*__mptr = (ptr);    \
             (type *)((char *)__mptr - offsetof(type, member)); })

static const uint8_t ICCID_List[][20+1] = {
    {"IMEI1234567890AB"},
    {"IMEI1234567890AC"},
    {"VIN1234567890ABCDEF1"},
};
#if 0
static const uint8_t VIN_List[][17+1] = {
    {"VIN1234567890ABC"},
    {"VIN1234567890ABD"},
    {"VIN0123456789ABCD"},
};
#endif

static char filter_vin[32] = "";
static uint16_t filter_vin_size=0;
static char filter_sn[32] = "";
static uint16_t filter_sn_size=0;
//char log_path[128] = "./log/dame.txt";

void ngx_filter_vin(const char* vin)
{
    filter_vin_size = (uint16_t)strlen(vin);
    memcpy(filter_vin, vin, filter_vin_size);
}
void ngx_filter_sn(const char* sn)
{
    filter_sn_size = (uint16_t)strlen(sn);
    memcpy(filter_sn, sn, filter_sn_size);
}

static int LogPrint(const int print, const char *__format, ...)
{
    static char text[1024*10];
    //static char wbuffer[1024*10];
    //static int windex=0; // 写指针
    //static uint32_t seek=0;
    //int _size=0;
    //FILE* fd=NULL;
    va_list ap;
    va_start(ap, __format);
    //vprintf(__format, ap);
    memset(text, 0, sizeof (text));
    //snprintf(text, sizeof (text), __format, ap);
    vsprintf(text, __format, ap);
    va_end(ap);
    //if(print) printf(text); // fflush(stdout);
    if(print) printf("%s", text); // fflush(stdout);
    /*_size = strlen (text);
    memcpy(&wbuffer[windex], text, _size);
    windex += _size;
    if(windex>1024)
    {
        if(0==seek)
        {
            fd = fopen(log_path, "w");
            fclose(fd);
        }
        fd = fopen(log_path, "ab");
        if(NULL!=fd)
        {
            fwrite(wbuffer, windex, 1, fd);
            fclose(fd);
            memset(wbuffer, 0, sizeof (wbuffer));
            seek = windex;
            windex = 0;
        }
        fflush(stdout);
    }*/
    return 0;
}


static struct shanghai_login login_pack;
static int handle_request_login(const int print, const struct general_pack_shanghai* const _pack)
{
    const struct shanghai_login *const request = (const struct shanghai_login *const)(_pack->data);
    uint8_t index = 0;
    int len=0;
#if 0
    // VIN
    len = (sizeof (VIN_List)/sizeof (VIN_List[0]));
    for (index=0; index<len; index++)
    {
        if(0==memcmp(VIN_List[index], _pack->VIN, sizeof(_pack->VIN)-1)) break;
    }
    if(index>=len)
    {
        return -1;
    }
    printf("SIM 卡号:%s\n", ICCID_List[index]);
#endif
    // ICCID
    len = (sizeof (ICCID_List)/sizeof (ICCID_List[0]));
    for (index=0; index<len; index++)
    {
        if(0==memcmp(ICCID_List[index], request->ICCID, sizeof (request->ICCID)-1)) break;
    }
//    if(index>=len)
//    {
//        return -1;
//    }
    //printf("登入时间:%s\n", request->UTC);
    LogPrint(print, "登入时间:%04d.%02d.%02d %02d:%02d:%02d\n", request->UTC[0]+2000, request->UTC[1], request->UTC[2]\
            , request->UTC[3], request->UTC[4], request->UTC[5]);
    LogPrint(print, "登入流水号:%d\n", request->count);
    LogPrint(print, "SIM 卡号:%s\n", request->ICCID);
    memcpy(&login_pack, request, sizeof (login_pack));   // save login info
    //pr_debug("Login : %s \n", request->UTC);
    fflush(stdout);
    return 0;
}
static int handle_request_logout(const int print, const struct general_pack_shanghai* const _pack)
{
    const struct shanghai_logout *const msg = (const struct shanghai_logout *const)(_pack->data);
    //if(msg->count != login_pack.count) return -1;
    //pr_debug("Logout : %s \n", msg->UTC);
    LogPrint(print, "登出时间:%04d.%02d.%02d %02d:%02d:%02d\n", msg->UTC[0]+2000, msg->UTC[1], msg->UTC[2]\
            , msg->UTC[3], msg->UTC[4], msg->UTC[5]);
    LogPrint(print, "登出流水号:%d\n", msg->count);
    fflush(stdout);
    return 0;
}

static int handle_report_real(int* const print, struct obd_agree_obj* const _obd_fops, const struct general_pack_shanghai *const _pack, struct device_list* const device, char _rbuf[], const unsigned int _rsize, struct data_base_obj* const _db_report)
{
    const struct shanghai_report_real *const msg = (const struct shanghai_report_real *const)(_pack->data);
    uint16_t index=0;
    int rlen = 0;
    int i=0;
    struct report_head* nmsg=NULL;  // next msg
    const struct report_head* fault=NULL;
    const struct shanghai_data_obd *obd=NULL;
    const struct shanghai_data_stream *stream=NULL;
    const struct shanghai_data_att *att=NULL;
    char data[7][128];
    char _buffer[512];

    LogPrint(*print, "Report [%d]: Time: %04d.%02d.%02d %02d:%02d:%02d \n", msg->count, msg->UTC[0]+2000, msg->UTC[1], msg->UTC[2]\
            , msg->UTC[3], msg->UTC[4], msg->UTC[5]);
        //fflush(stdout);
    memset(_buffer, 0, sizeof (_buffer));
    sprintf(_buffer, "%04d.%02d.%02d %02d:%02d:%02d", msg->UTC[0]+2000, msg->UTC[1], msg->UTC[2]\
            , msg->UTC[3], msg->UTC[4], msg->UTC[5]);
    _db_report->fops->insert_item_string(_db_report, "UTC", _buffer);
    // 消息，可包含多条
    nmsg = msg->msg;
    //pr_debug("msg.msg : %d \n", (NULL!=msg->msg)); fflush(stdout);
    rlen = 0;
    while(NULL!=nmsg)
    {
        //pr_debug("type_msg : %d \n", nmsg->type_msg); fflush(stdout);
        switch (nmsg->type_msg)
        {
            // 1） OBD 信息数据格式和定义见表 A.6 所示。
            case MSG_OBD:       // 0x01  OBD 信息
                obd = (const struct shanghai_data_obd *) container_of(nmsg, struct shanghai_data_obd, head);
                // 序列号过滤信息
                if((filter_sn_size>0) && (0==strncmp((char*)(&obd->SVIN[6]), filter_sn, filter_sn_size)) )
                {
                    *print = 1;
                }
                if((filter_sn_size>0) && (0==strncmp((char*)(&obd->CVN[6]), filter_sn, filter_sn_size)) )
                {
                    *print = 1;
                }
                //if(*print)
                {
                    memset(_buffer, 0, sizeof (_buffer));
                    LogPrint(*print, "OBD 诊断协议: %d (“0”代表 IOS15765，“1”代表IOS27145，“2”代表 SAEJ1939，“0xFE”表示无效)\n", obd->protocol);
                    LogPrint(*print, "MIL 状态: %d (“0”代表未点亮，“1”代表点亮。“0xFE”表示无效)\n", obd->MIL);
                    memset(_buffer, 0, sizeof (_buffer));
                    //itoa(obd->status, _buffer, 2);
                    //printf("诊断支持状态: %d %s\n", obd->status, _buffer);
                    LogPrint(*print, "诊断支持状态: %d \n", obd->status);
                    memset(_buffer, 0, sizeof (_buffer));
                    //itoa(obd->ready, _buffer, 2);
                    //printf("诊断就绪状态: %d %s\n", obd->ready, _buffer);
                    LogPrint(*print, "诊断就绪状态: %d \n", obd->ready);
                    memset(data, 0, sizeof (data));
                    CarInfoSearch(CarListPath, (char*)obd->VIN, data[0]);
                    LogPrint(*print, "车辆识别码（VIN）: %s 车型：%s 车牌:%s \n", obd->VIN, data[1], data[6]);
                    LogPrint(*print, "软件标定识别号: %s \n", obd->SVIN);
                    if(17==strlen(obd->VIN))
                    {
                        memset(device->VIN, 0, sizeof (device->VIN));
                        memcpy(device->VIN, obd->VIN, sizeof (obd->VIN));
                    }
                    LogPrint(*print, "标定验证码（CVN）: %s \n", obd->CVN);
                    if(17==strlen(obd->VIN)) _db_report->fops->insert_item_string(_db_report, "vin", obd->VIN);
                    else _db_report->fops->insert_item_string(_db_report, "vin", "11112222333344445");
                    _db_report->fops->insert_item_int(_db_report, "prot", obd->protocol);
                    _db_report->fops->insert_item_int(_db_report, "mil", obd->MIL);
                    _db_report->fops->insert_item_int(_db_report, "status", obd->status);
                    _db_report->fops->insert_item_int(_db_report, "ready", obd->ready);
                    _db_report->fops->insert_item_string(_db_report, "svin", obd->SVIN);
                    _db_report->fops->insert_item_string(_db_report, "cin", obd->CVN);
                    LogPrint(*print, "标定验证码（CVN）: %s  match_fw=%d fw:0x%04X\n", obd->CVN, match_fw(obd->CVN), device->fw_update);
                    //if((0!=match_fw(obd->CVN)) || ((0==match_fw(obd->CVN)) && (0x9911 != device->fw_update)))
                    if(0x9911 != device->fw_flag) // 记录固件信息
                    {
                        device->fw_flag = 0x9911;
                        sql_insert_fw_update(obd->VIN, ".bin", "Old_FW", obd->CVN, &obd->SVIN[6], match_fw(obd->CVN)); // 旧固件标记
                    }
                    if((0!=match_fw(obd->CVN)) && (device->fw_update>(100/10)))  // 每一百秒推送一次更新
                    {
                        // 推送更新,发送推送更新指令
                        struct upload* _load = NULL;
                        uint8_t buffer[sizeof(struct upload)];
                        int len = 0;
                        device->fw_update=0;
                        device->fw_flag=0;
                        _load = upload_init(UPLOAD_PUSH, 0, 10*1024, get_fw_crc(), "OBD-4G", "000000000000", 12);
                        len = upload_encode(_load, buffer, sizeof (buffer));
                        len = _obd_fops->fops->base->pack.encode(_obd_fops, buffer, len, obd_buf, sizeof (obd_buf));
                        //if(NULL != csend) csend(device->socket, obd_buf, len); // 发送数据到客户端
                        if(_rsize>=len)
                        {
                            rlen = len;
                            memcpy(_rbuf, obd_buf, rlen);
                        }
                    }
                    if(device->fw_update<1000) device->fw_update++;


                    memset(_buffer, 0, sizeof (_buffer));
                    memcpy(_buffer, obd->IUPR, 36);
                    LogPrint(*print, "MSG_OBD IUPR: %s \n", (char*)(_buffer));
                    LogPrint(*print, "IUPR: ");
                    memset(_buffer, 0, sizeof (_buffer));
                    for(i=0; i<18; i++)
                    {
                        LogPrint(*print, " %04X",obd->IUPR[i]);
                        sprintf(&_buffer[strlen(_buffer)], "%04X ",obd->IUPR[i]);
                    }
                    _db_report->fops->insert_item_string(_db_report, "iupr", _buffer);
                    LogPrint(*print, "\n");
                    LogPrint(*print, "故障码总数: %d \n", obd->fault_total);
                    fault = &(obd->fault_list);
                    LogPrint(*print, "故障码: ");  // 故障码, BYTE（4）
                    _db_report->fops->insert_item_int(_db_report, "fault_total", obd->fault_total);
                    memset(_buffer, 0, sizeof (_buffer));
                    //while(NULL!=fault)
                    for(index=0; index<obd->fault_total; index++)
                    {
                        sprintf(&_buffer[strlen(_buffer)], "%04X ",fault->data);
                        LogPrint(*print, " 0x%02X", fault->data);  // 故障码, BYTE（4）
                        fault = fault->next;  // 下一个数据
                        if(NULL==fault) break;
                    }
                    _db_report->fops->insert_item_string(_db_report, "fault", _buffer);
                    LogPrint(*print, "\n"); // fflush(stdout);}
                }
                break;
            // 2）数据流信息数据格式和定义见表 A.7 所示，补充数据流信息数据格式和定义见表 A.8 所示。
            case MSG_STREAM:     // 0x02  数据流信息
                stream = (const struct shanghai_data_stream *)container_of(nmsg, struct shanghai_data_stream, head);
                //if(*print)
                {
                    LogPrint(*print, "车速: %4d [%5.4f] (0~250.996km/h)\n", stream->speed, FloatConvert(stream->speed, 0, 1.0f/256));
                    LogPrint(*print, "大气压力: %4d [%5.4f] (0~125kPa)\n", stream->kPa, FloatConvert(stream->kPa, 0, 0.5f));
                    LogPrint(*print, "发动机净输出扭矩: %4d [%4d] (-125~125 %% “0xFF”表示无效)\n", stream->Nm, IntConvert(stream->Nm, -125, 1));
                    LogPrint(*print, "摩擦扭矩: %4d [%4d] (-125~125%%)\n", stream->Nmf, IntConvert(stream->Nmf, -125, 1));
                    LogPrint(*print, "发动机转速: %4d [%5.4f] (0~8031.875rpm)\n", stream->rpm, FloatConvert(stream->rpm, 0, 0.125f));
                    LogPrint(*print, "发动机燃料流量: %4d [%5.4f] (0~3212.75L/h)\n", stream->Lh, FloatConvert(stream->Lh, 0, 0.05f));
                    LogPrint(*print, "SCR 上游 NOx 传感器输出值: %4d [%5.4f] (-200~3212.75ppm)\n", stream->ppm_up, FloatConvert(stream->ppm_up, -200, 0.05f));
                    LogPrint(*print, "SCR 下游 NOx 传感器输出值: %4d [%5.4f] (-200~3212.75ppm)\n", stream->ppm_down, FloatConvert(stream->ppm_down, -200, 0.05f));
                    LogPrint(*print, "反应剂余量: %4d [%5.4f] (0~100%%)\n", stream->urea_level, FloatConvert(stream->urea_level, 0, 0.4f));
                    LogPrint(*print, "进气量 : %4d [%5.4f] (0~3212.75ppm)\n", stream->kgh, FloatConvert(stream->kgh, 0, 0.05f));
                    LogPrint(*print, "SCR 入口温度（后处理上游排气温度）: %4d [%5.4f] (-273~1734.96875℃)\n", stream->SCR_in, FloatConvert(stream->SCR_in, -273, 0.03125f));
                    LogPrint(*print, "SCR 出口温度（后处理下游排气温度）: %4d [%5.4f] (-273~1734.96875℃)\n", stream->SCR_out, FloatConvert(stream->SCR_out, -273, 0.03125f));
                    LogPrint(*print, "DPF 压差（或 DPF排气背压）: %4d [%5.4f] (0~6425.5 kPa)\n", stream->DPF, FloatConvert(stream->DPF, 0, 0.1f));
                    LogPrint(*print, "发动机冷却液温度: %4d [%5.4f] (-40~210℃)\n", stream->coolant_temp, FloatConvert(stream->coolant_temp, -40, 1.0f));
                    LogPrint(*print, "油箱液位: %4d [%5.4f] (0~100%%)\n", stream->tank_level, FloatConvert(stream->tank_level, 0, 0.4f));
                    LogPrint(*print, "定位状态: %3d \n", stream->gps_status);
                    LogPrint(*print, "经度: %4d [%5.4f] (0~180.000000°)\n", stream->longitude, DIntConvert(stream->longitude, 0, 0.000001f));
                    LogPrint(*print, "纬度: %4d [%5.4f] (0~180.000000°)\n", stream->latitude, DIntConvert(stream->latitude, 0, 0.000001f));
                    LogPrint(*print, "累计里程: %4d [%5.4f] (精度：0.1km)\n", stream->mileages_total, DIntConvert(stream->mileages_total, 0, 0.1f));
                    //fflush(stdout);
                    _db_report->fops->insert_item(_db_report, "speed", FloatConvert(stream->speed, 0, 1.0f/256));
                    _db_report->fops->insert_item(_db_report, "kpa", FloatConvert(stream->kPa, 0, 0.5f));
                    _db_report->fops->insert_item(_db_report, "nm", IntConvert(stream->Nm, -125, 1));
                    _db_report->fops->insert_item(_db_report, "nmf", IntConvert(stream->Nmf, -125, 1));
                    _db_report->fops->insert_item(_db_report, "rpm", FloatConvert(stream->rpm, 0, 0.125f));
                    _db_report->fops->insert_item(_db_report, "Lh", FloatConvert(stream->Lh, 0, 0.05f));
                    _db_report->fops->insert_item(_db_report, "ppm_up", FloatConvert(stream->ppm_up, -200, 0.05f));
                    _db_report->fops->insert_item(_db_report, "ppm_down", FloatConvert(stream->ppm_down, -200, 0.05f));
                    _db_report->fops->insert_item(_db_report, "urea", FloatConvert(stream->urea_level, 0, 0.4f));
                    _db_report->fops->insert_item(_db_report, "kgh", FloatConvert(stream->kgh, 0, 0.05f));
                    _db_report->fops->insert_item(_db_report, "SCR_in", FloatConvert(stream->SCR_in, -273, 0.03125f));
                    _db_report->fops->insert_item(_db_report, "SCR_out", FloatConvert(stream->SCR_out, -273, 0.03125f));
                    _db_report->fops->insert_item(_db_report, "DPF", FloatConvert(stream->DPF, 0, 0.1f));
                    _db_report->fops->insert_item(_db_report, "coolant_temp", FloatConvert(stream->coolant_temp, -40, 1.0f));
                    _db_report->fops->insert_item(_db_report, "tank", FloatConvert(stream->tank_level, 0, 0.4f));
                    _db_report->fops->insert_item_int(_db_report, "gps_status", stream->gps_status);
                    _db_report->fops->insert_item(_db_report, "lon", DIntConvert(stream->longitude, 0, 0.000001f));
                    _db_report->fops->insert_item(_db_report, "lat", DIntConvert(stream->latitude, 0, 0.000001f));
                    _db_report->fops->insert_item(_db_report, "mil_total", DIntConvert(stream->mileages_total, 0, 0.1f));
                }
                //MySqlSeve(_pack->VIN, DIntConvert(stream->longitude, 0, 0.000001f), DIntConvert(stream->latitude, 0, 0.000001f));
                break;
            case MSG_STREAM_ATT: // 0x80  补充数据流
                att = (const struct shanghai_data_att *)container_of(nmsg, struct shanghai_data_att, head);
                //if(*print)
                {
                    LogPrint(*print, "发动机扭矩模式 : %3d (0：超速失效 1：转速控制 2：扭矩控制 3：转速/扭矩控制 9：正常)\n", att->Nm_mode);
                    LogPrint(*print, "油门踏板: %3d [%5.4f] (0~100%%)\n", att->accelerator, FloatConvert(att->accelerator, 0, 0.4f));
                    LogPrint(*print, "累计油耗: %3d [%5.4f] (0~2 105 540 607.5L)\n", att->oil_consume, DIntConvert(att->oil_consume, 0, 0.5f));
                    LogPrint(*print, "尿素箱温度: %3d [%5.4f] (-40~210℃)\n", att->urea_tank_temp, FloatConvert(att->urea_tank_temp, -40, 1.0f));
                    LogPrint(*print, "实际尿素喷射量: %3d [%5.4f] (精度：0.01 ml/h per bit)\n", att->mlh_urea_actual, DIntConvert(att->mlh_urea_actual, 0, 0.01f));
                    LogPrint(*print, "累计尿素消耗 : %3d [%5.4f] (精度：1 g per bit)\n", att->mlh_urea_total, DIntConvert(att->mlh_urea_total, 0, 1.0f));
                    LogPrint(*print, "DPF 排气温度 : %3d [%5.4f] (-273~1734.96875℃)\n", att->exit_gas_temp, FloatConvert(att->exit_gas_temp, -273, 0.03125f));
                    //fflush(stdout);
                    _db_report->fops->insert_item_int(_db_report, "Nm_mode", att->Nm_mode);
                    _db_report->fops->insert_item(_db_report, "accelerator", FloatConvert(att->accelerator, 0, 0.4f));
                    _db_report->fops->insert_item(_db_report, "oil_consume", DIntConvert(att->oil_consume, 0, 0.5f));
                    _db_report->fops->insert_item(_db_report, "urea_temp", FloatConvert(att->urea_tank_temp, -40, 1.0f));
                    _db_report->fops->insert_item(_db_report, "urea_actual", DIntConvert(att->mlh_urea_actual, 0, 0.01f));
                    _db_report->fops->insert_item(_db_report, "urea_total", DIntConvert(att->mlh_urea_total, 0, 1.0f));
                    _db_report->fops->insert_item(_db_report, "gas_temp", FloatConvert(att->exit_gas_temp, -273, 0.03125f));
                    _db_report->fops->insert_item_int(_db_report, "version", device->crc);
                }
                break;
            // 0x03-0x7F  预留
            // 0x81~0xFE  用户自定义
            default:
                break;
        }
        nmsg = nmsg->next;   // 下一个数据
    }
    fflush(stdout);
    return rlen;
}

static char* load_file(const char* filename, long* const size)
{
    static char filebin[2*1024*1024]; // 2MB
    long _size=0;
    size_t count=0;
    FILE* fd = fopen(filename, "r");
    if(NULL == fd)
    {
        printf("file %s not exist!..3 \n", filename);  fflush(stdout);
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

static int upload_query(int* const print, const char* const vin, struct obd_agree_obj* const _obd_fops, struct upload* const decode, const char* suffix, struct device_list* const device, char _rbuf[], const unsigned int _rsize)
{
    int len;
    char filename[128];
    uint8_t buffer[sizeof(struct upload)];
    int rlen = 0;
    char* fw;
    uint32_t checksum=0;
    long _size=0;
    int upload_flag=0;
    char Firmware[64];
    char Config[64];
    char Des[512];
    const char* file=NULL;
    if((filter_sn_size>0) && (0==strcmp((char*)(decode->data), filter_sn)))
    {
        *print = 1;
    }
    memcpy(device->sn, decode->data, decode->data_len);  // save sn
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
    device->crc = decode->checksum;
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
            //if(*print)
            {
                LogPrint(*print, "read file %s fail!\n", filename); fflush(stdout);
            }
            goto err;
        }
        // 计算和校验
        //checksum = fast_crc16(0, (unsigned char *)fw, decode->pack_index);
        checksum = fast_crc16(0, (unsigned char *)fw, _size);
    }
    //if(*print)
    {
        LogPrint(*print, "%s@%d flag:%d Firmware:%s Config:%s\n", __func__, __LINE__, upload_flag, Firmware, Config);
        LogPrint(*print, "%s@%d file:%s serial number[%02d]: %s Model:%s\n", __func__, __LINE__, filename, decode->data_len, decode->data, decode->Model);
        LogPrint(*print, "%s@%d fread checksum:0x%04X decode->checksum:0x%04X\n", __func__, __LINE__, checksum, decode->checksum);
    }
    //upload_flag=1;  // 强制更新
    memset(Des, 0, sizeof(Des));
    snprintf(Des, sizeof (Des)-1, "update file[%s] serial number[%02d]: %s Model:%s checksum:0x%04X decode->checksum:0x%04X", filename, decode->data_len, decode->data, decode->Model, checksum, decode->checksum);
    sql_insert_fw_update(vin, suffix, Des, filename, decode->data, decode->checksum);
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
    len = _obd_fops->fops->base->pack.encode(_obd_fops, buffer, len, obd_buf, sizeof (obd_buf)); // encode
    //if(NULL != csend) csend(device->socket, obd_buf, len); // 发送数据到客户端
    if(_rsize>=len)
    {
        rlen = len;
        memcpy(_rbuf, obd_buf, rlen);
    }
    return rlen;
}
//void upload_download(struct obd_agree_obj* const _obd_fops, struct upload* const decode, const char* suffix, const int client, void(*csend)(const int sockfd, const void *buf, const size_t len))
static int upload_download(const int print, struct obd_agree_obj* const _obd_fops, struct upload* const decode, struct device_list* const device, char _rbuf[], const unsigned int _rsize)
{
    int len;
    char filename[128];
    const char *suffix=NULL;
    uint8_t buffer[sizeof(struct upload)];
    int rlen = 0;
    char* fw;
    uint32_t checksum=0;
    long _size=0;
    if(decode->data_len<strlen("./*.bin")+1)
    {
        LogPrint(print, "decode data_len[%d] error!\n", decode->data_len); fflush(stdout);
        //return;
        _size = 0; // 返回错误
        decode->data_len = 0;
        goto err;
    }
    LogPrint(print, "%s@%d file:%s\n", __func__, __LINE__, decode->data);
    LogPrint(print, "%s@%d total:%d seek:%d\n", __func__, __LINE__, decode->pack_total, decode->pack_index);
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
        LogPrint(print, "read file %s fail!\n", filename); fflush(stdout);
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
    len = _obd_fops->fops->base->pack.encode(_obd_fops, buffer, len, obd_buf, sizeof (obd_buf)); // encode
    //if(NULL != csend) csend(device->socket, obd_buf, len); // 发送数据到客户端
    if(_rsize>=len)
    {
        rlen = len;
        memcpy(_rbuf, obd_buf, rlen);
    }
    return rlen;
}

static int handle_request_userdef(int* const print, struct obd_agree_obj* const _obd_fops, const struct general_pack_shanghai* const _pack, struct device_list* const device, char _rbuf[], const unsigned int _rsize)
{
    const struct shanghai_userdef *const msg = (const struct shanghai_userdef *const)(_pack->data);
    struct upload decode;
    int rlen = 0;
    int len;
    memset(&decode, 0, sizeof(struct upload));
    len = upload_decode(&decode, msg->data, msg->_dsize);
    rlen = 0;
    if(len<0)
    {
        LogPrint(*print, "userdef decode error!\n"); fflush(stdout);
    }
    else
    {
#if 0
        LogPrint(*print, "decode cmd : 0x%02X\n", decode.cmd); fflush(stdout);
        LogPrint(*print, "decode total : %d\n", decode.pack_total); fflush(stdout);
        LogPrint(*print, "decode index : %d\n", decode.pack_index); fflush(stdout);
        LogPrint(*print, "decode checksum : 0x%02X\n", decode.checksum); fflush(stdout);
        LogPrint(*print, "decode Model : %s\n", decode.Model); fflush(stdout);
        LogPrint(*print, "decode len : %d\n", decode.data_len); fflush(stdout);
        LogPrint(*print, "decode data : %s\n", decode.data); fflush(stdout);
        LogPrint(*print, "decode CRC : 0x%02X\n", decode.CRC); fflush(stdout);
#endif
        switch (decode.cmd)
        {
            case UPLOAD_LOGIN:  // 登录
                LogPrint(*print, "userdef Login\n"); fflush(stdout);
                break;
            case UPLOAD_VIN:
                {
                    static const char vin_path[] = "./upload/vin.list";
                    char vin[32];
                    uint8_t buffer[sizeof(struct upload)];
                    memset(vin, 0, sizeof (vin));
                    decode.pack_total = 0;
                    decode.pack_index = 0;
                    decode.data_len = 0;
                    decode.checksum = 0;
                    printf("SN:%s\n", decode.data);
                    if(0==create_vin_search(vin_path, decode.data, vin))
                    {
                        printf("数据获取成功\n");
                        decode.data_len = strlen(vin);
                        memcpy(decode.data, vin, decode.data_len);
                    }
                    printf("VIN:%s VIN:%s\n", decode.data, vin);
                    memset(buffer, 0, sizeof (buffer));
                    len = upload_encode(&decode, buffer, sizeof (buffer));   // user def
                    len = _obd_fops->fops->base->pack.encode(_obd_fops, buffer, len, obd_buf, sizeof (obd_buf)); // encode
                    //if(NULL != csend) csend(device->socket, obd_buf, len); // 发送数据到客户端
                    if(_rsize>=len)
                    {
                        rlen = len;
                        memcpy(_rbuf, obd_buf, rlen);
                    }
                }
                break;
            case UPLOAD_QUERY_CFG:    // 查询
                LogPrint(*print, "userdef Config [cfg]\n"); fflush(stdout);
                printf("userdef Config [cfg]\n"); fflush(stdout);
                rlen = upload_query(print, _pack->VIN, _obd_fops,  &decode, ".cfg", device, _rbuf, _rsize);
                break;
            case UPLOAD_DOWNLOAD:   // 下载
                LogPrint(*print, "userdef DownLoad\n"); fflush(stdout);
                printf("userdef DownLoad\n"); fflush(stdout);
                rlen = upload_download(*print, _obd_fops,  &decode, device, _rbuf, _rsize);
                break;
            case UPLOAD_QUERY_FW:    // 查询
                LogPrint(*print, "userdef Firmware [bin]\n"); fflush(stdout);
                printf("userdef Firmware [bin]\n"); fflush(stdout);
                rlen = upload_query(print, (char*)_pack->VIN, _obd_fops,  &decode, ".bin", device, _rbuf, _rsize);
                break;
            case UPLOAD_GET_TIME:        // 校时
                LogPrint(*print, "userdef Get Time\n"); fflush(stdout);
                {
                    uint8_t buffer[sizeof(struct upload)];
                    decode.cmd = UPLOAD_GET_TIME;
                    decode.pack_total = (uint32_t)time(NULL);  // UTC time
                    decode.data_len = 0;
                    memset(buffer, 0, sizeof (buffer));
                    len = upload_encode(&decode, buffer, sizeof (buffer));   // user def
                    len = _obd_fops->fops->base->pack.encode(_obd_fops, buffer, len, obd_buf, sizeof (obd_buf)); // encode
                    //if(NULL != csend) csend(device->socket, obd_buf, len); // 发送数据到客户端
                    if(_rsize>=len)
                    {
                        rlen = len;
                        memcpy(_rbuf, obd_buf, rlen);
                    }
                }
                break;
            default:
                break;
        }
    }
    return rlen;
}
static int protocol_shanghai(int* const print, int* relay, struct obd_agree_obj* const _obd_fops, const struct general_pack_shanghai* const _pack, struct device_list* const device, char _rbuf[], const unsigned int _rsize, struct data_base_obj* const _db_report)
{
    const char* ssl_type[] = {
        "0x00: NULL",
        "0x01：数据不加密",
        "0x02：数据经过 RSA 算法加密",
        "0x03：数据经过国密 SM2 算法加密",
        "“0xFE”标识异常，“0xFF”表示无效，其他预留 ",
    };
    int len = 0;
    const struct general_pack_shanghai* const pack = _pack;
    if((filter_vin_size>0) && (0==strcmp((char*)(pack->VIN), filter_vin)))
    {
        *print = 1;
    }
    if((0==filter_vin_size) && (0==filter_sn_size))
    {
        *print = 1;
    }
    //if(*print)
    {
        LogPrint(*print, "起始符: %c%c \n", pack->start[0], pack->start[1]);
        LogPrint(*print, "命令单元: %d \n", pack->cmd);
        LogPrint(*print, "车辆识别号: %s\n", pack->VIN);
        LogPrint(*print, "软件版本号: %d \n", pack->soft_version);
        LogPrint(*print, "数据加密方式: %s \n", ssl_type[pack->ssl&0x3]);
        LogPrint(*print, "数据单元长度: %d \n", pack->data_len);
        LogPrint(*print, "校验码: %02X \n", pack->BCC);
        fflush(stdout);
    }
    if(17==strlen(pack->VIN))
    {
        memset(device->VIN, 0, sizeof (device->VIN));
        memcpy(device->VIN, pack->VIN, sizeof (pack->VIN));
    }
    *relay = 1;
    len = 0;
    switch(pack->cmd)
    {
        case CMD_LOGIN:        // 车辆登入
            LogPrint(*print, "车辆登入\n"); fflush(stdout);
            handle_request_login(*print, pack);
            break;
        case CMD_REPORT_REAL:  // 实时信息上报
            LogPrint(*print, "实时信息上报\n"); fflush(stdout);
            len = handle_report_real(print, _obd_fops,  pack, device, _rbuf, _rsize, _db_report);
            break;
        case CMD_REPORT_LATER: // 补发信息上报
            LogPrint(*print, "补发信息上报\n"); fflush(stdout);
            len = handle_report_real(print, _obd_fops,  pack, device, _rbuf, _rsize, _db_report);
            break;
        case CMD_LOGOUT:       // 车辆登出
            LogPrint(*print, "车辆登出\n"); fflush(stdout);
            handle_request_logout(*print, pack);
            break;
        case CMD_UTC:          // 终端校时
            LogPrint(*print, "终端校时\n"); fflush(stdout);
            break;
        case CMD_USERDEF:      // 用户自定义
            LogPrint(*print, "用户自定义\n"); fflush(stdout);
            len = handle_request_userdef(print, _obd_fops,  pack, device, _rbuf, _rsize);
            *relay = 0;
            break;
        default:
            //if(*print)
            {
                LogPrint(*print, "default\n"); fflush(stdout);
            }
            *relay = 0;
            break;
    }
    return len;
}
#include "DateTime.h"
#include <stdio.h>
static void UTC2file(const uint32_t times, void* const buf, const size_t _size)
{
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
    snprintf((char *)buf, (size_t)_size, "./log/jsons-%d-%.2d-%.2d-%02d%02d%02d.txt", localtime.year, localtime.month, localtime.day, localtime.hour, localtime.minute, localtime.second);
}
static void UTC2Format(const uint32_t times, uint8_t buf[], const size_t _size)
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
int ngx_decode_server(int* const print, struct obd_agree_obj* const _obd_fops, const uint8_t pack[], const uint16_t _psize, void* const _msg_buf, const uint16_t _msize, struct device_list* const device, void(*csend)(const int sockfd, const void *buf, const uint16_t len))
{
    int len = 0;
    char filename[128];
    int relay=0;
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
#if 0
    for(len=0; len<40; len++)
    {
        printf("\033[1A"); //先回到上一行
        printf("\033[K");  //清除该行
    }
#endif
    len =_obd_fops->fops->decode(_obd_fops, pack, _psize, _msg_buf, _msize);
    if(*print)
    {
        printf("decode_test len : %d \n", len); fflush(stdout);
    }
    if(len<0) return -1;
    device->type = _obd_fops->fops->protocol;//();
    _db_report.fops->init(&_db_report); // MySqlInit();
    switch (_obd_fops->fops->protocol) // switch (_agree_ofp->protocol())
    {
        case PRO_TYPE_CCU:    // CCU
            //
            break;
        case PRO_TYPE_SHH:    // ShangHai
            if(*print)
            {
                printf("协议类型：上海OBD协议 \n"); fflush(stdout);
            }
            protocol_shanghai(print, &relay, _obd_fops,  (const struct general_pack_shanghai* const)_msg_buf, device, _msg_buf, 0, &_db_report);
            break;
        default:
            break;
    }
    //insert_sql();
    /*ret = */_db_report.fops->insert_sql(&_db_report/*, _conn_ofp*/);
    _db_report.fops->clear(&_db_report);
    _db_report.fops->close(&_db_report); // MySqlClose();
    // 消息转发
    //printf("relay_fd:%d relay flag : %d \n", device->relay_fd, relay); fflush(stdout);
    if((device->relay_fd>=0) && (1==relay)) relay_msg(device->relay_fd, pack, len);
    // save log
    UTC2Format(time(NULL), (uint8_t *)device->UTC, 6);
    if(device->save_log)
    {
        time_t timer;
        memset(filename, 0, sizeof (filename));
        if(strlen(device->sn)>=12)
        {
            snprintf(filename, sizeof (filename)-1, "./log/jsons-%s.txt", device->sn);
        }
        else if(strlen(device->VIN)>=8)
        {
            snprintf(filename, sizeof (filename)-1, "./log/jsons-%s.txt", device->VIN);
        }
        else
        {
            timer = time(NULL);
            UTC2file(timer, filename, sizeof(filename));
        }
        //int json_device_save(const char* filename, const struct device_list* const device, const uint8_t pack[], const uint16_t _psize)
        json_device_save(filename, device, pack, len);
    }
    else
    {
        json_device_save(NULL, device, pack, len);
    }
    return 0;
}
//static int __frame_decode(const struct agreement_ofp *const _agree_obd, const char _frame[], unsigned int _fsize, char _buf[], const unsigned int _bsize)
int ngx_server(int* const print, struct obd_agree_obj* const _obd_fops, const uint8_t _frame[], const uint16_t _fsize, void* const _msg_buf, const uint16_t _msize, struct device_list* const device, char _rbuf[], const unsigned int _rsize, struct data_base_obj* const _db_report)
{
    int len = 0;
    int relay = 0;
    len =_obd_fops->fops->decode(_obd_fops, _frame, _fsize, _msg_buf, _msize);
    printf("decode_test len : %d \n", len); fflush(stdout);
    if(len<0) return -1;
    device->type = _obd_fops->fops->protocol;//();
    switch (_obd_fops->fops->protocol) // switch (_agree_ofp->protocol())
    {
        case PRO_TYPE_CCU:    // CCU
            //
            break;
        case PRO_TYPE_SHH:    // ShangHai
            printf("协议类型：上海OBD协议 \n"); fflush(stdout);
            return protocol_shanghai(print, &relay, _obd_fops,  (const struct general_pack_shanghai* const)_msg_buf, device, _rbuf, _rsize, _db_report);
            //break;
        default:
            break;
    }
    return 0;
}





