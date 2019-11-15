/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : agreement_test.h
* Author             : Merafour
* Last Modified Date : 11/12/2019
* Description        : OBD agreement test.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>

#include <time.h>
#include "agreement.h"
#include "obd_agree_shanghai.h"
#include "DateTime.h"
#include "storage_pack.h"
#include "upload.h"

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
static const uint8_t VIN_List[][17+1] = {
    {"VIN1234567890ABC"},
    {"VIN1234567890ABD"},
    {"VIN0123456789ABCD"},
};

static struct shanghai_login login_pack;
static char filename[128]="NULL";

static int handle_request_login(const struct general_pack_shanghai* const _pack)
{
    const struct shanghai_login *const request = (const struct shanghai_login *const)(_pack->data);
    uint8_t index = 0;
    int len=0;
    // VIN
    len = (sizeof (ICCID_List)/sizeof (ICCID_List[0]));
    for (index=0; index<len; index++)
    {
        if(0==memcmp(VIN_List[index], _pack->VIN, sizeof(_pack->VIN)-1)) break;
    }
    if(index>=len)
    {
        return -1;
    }
    // ICCID
    len = (sizeof (ICCID_List)/sizeof (ICCID_List[0]));
    for (index=0; index<len; index++)
    {
        if(0==memcmp(ICCID_List[index], request->ICCID, sizeof (request->ICCID)-1)) break;
    }
    if(index>=len)
    {
        return -1;
    }
    memcpy(&login_pack, request, sizeof (login_pack));   // save login info
    pr_debug("Login : %s \n", request->UTC);
    fflush(stdout);
    time_t timer = time(NULL);
    DateTime      utctime   = {.year = 1970, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 0};
    DateTime      localtime = {.year = 1970, .month = 1, .day = 1, .hour = 8, .minute = 0, .second = 0};

    memset(filename, 0, sizeof (filename));
    if(timer > INT32_MAX)
    {
      utctime = GregorianCalendarDateAddSecond(utctime, INT32_MAX);
      utctime = GregorianCalendarDateAddSecond(utctime, (int)(timer - INT32_MAX));
    }
    else
    {
      utctime = GregorianCalendarDateAddSecond(utctime, (int)timer);
    }

    GregorianCalendarDateToModifiedJulianDate(utctime);
    localtime = GregorianCalendarDateAddHour(utctime, 8);
    snprintf(filename, sizeof (filename)-1, "%s-%d%d%d-%d-%d-%d.json", "VIN1234567890", localtime.year, localtime.month, localtime.day, localtime.hour, localtime.minute, localtime.second);
    json_table_update(filename, "ShangHai");
    json_ShangHai_update(filename, _pack);
    return 0;
}
static int handle_request_logout(const struct general_pack_shanghai* const _pack)
{
    const struct shanghai_logout *const msg = (const struct shanghai_logout *const)(_pack->data);
    if(msg->count != login_pack.count) return -1;
    pr_debug("Logout : %s \n", msg->UTC);
    fflush(stdout);
    json_ShangHai_update(filename, _pack);
    return 0;
}
static int handle_report_real(const struct general_pack_shanghai *const _pack)
{
    const struct shanghai_report_real *const msg = (const struct shanghai_report_real *const)(_pack->data);
    uint16_t index=0;
//    uint16_t data_len=0;
    struct report_head* nmsg=NULL;  // next msg
    const struct report_head* fault=NULL;
    const struct shanghai_data_obd *obd=NULL;
#if debug_log
    const struct shanghai_data_stream *stream=NULL;
    const struct shanghai_data_att *att=NULL;
#endif

    printf("Report [%d]: Time: %d.%d.%d %d:%d:%d \n", msg->count, msg->UTC[0]+2000, msg->UTC[1], msg->UTC[2]\
            , msg->UTC[3], msg->UTC[4], msg->UTC[5]);
    fflush(stdout);
    // 消息，可包含多条
    nmsg = msg->msg;
    //pr_debug("msg.msg : %d \n", (NULL!=msg->msg)); fflush(stdout);
    while(NULL!=nmsg)
    {
        //pr_debug("type_msg : %d \n", nmsg->type_msg); fflush(stdout);
        switch (nmsg->type_msg)
        {
            // 1） OBD 信息数据格式和定义见表 A.6 所示。
            case MSG_OBD:       // 0x01  OBD 信息
                obd = (const struct shanghai_data_obd *) container_of(nmsg, struct shanghai_data_obd, head);
                pr_debug("MSG_OBD protocol: %d \n", obd->protocol);
                pr_debug("MSG_OBD MIL: %d \n", obd->MIL);
                pr_debug("MSG_OBD status: %d \n", obd->status);
                pr_debug("MSG_OBD ready: %d \n", obd->ready);
                pr_debug("MSG_OBD VIN: %s \n", obd->VIN);
                pr_debug("MSG_OBD SVIN: %s \n", obd->SVIN);
                pr_debug("MSG_OBD CVN: %s \n", obd->CVN);
                //pr_debug("MSG_OBD IUPR: %d \n", obd->IUPR);
                pr_debug("MSG_OBD fault_total: %d \n", obd->fault_total);
                fault = &(obd->fault_list);
                pr_debug("故障码: ");  // 故障码, BYTE（4）
                for(index=0; index<obd->fault_total; index++)
                //while(NULL!=fault)
                {
                    pr_debug(" 0x%02X", fault->data);  // 故障码, BYTE（4）
                    fault = fault->next;  // 下一个数据
                    if(NULL==fault) break;
                }
                pr_debug("\n");fflush(stdout);
                break;
            // 2）数据流信息数据格式和定义见表 A.7 所示，补充数据流信息数据格式和定义见表 A.8 所示。
            case MSG_STREAM:     // 0x02  数据流信息
#if debug_log
                stream = (const struct shanghai_data_stream *)container_of(nmsg, struct shanghai_data_stream, head);
#endif
                pr_debug("MSG_STREAM speed: %d \n", stream->speed);
                pr_debug("MSG_STREAM kPa: %d \n", stream->kPa);
                pr_debug("MSG_STREAM Nm: %d \n", stream->Nm);
                pr_debug("MSG_STREAM Nmf: %d \n", stream->Nmf);
                pr_debug("MSG_STREAM rpm: %d \n", stream->rpm);
                pr_debug("MSG_STREAM Lh: %d \n", stream->Lh);
                pr_debug("MSG_STREAM ppm_down: %d \n", stream->ppm_down);
                pr_debug("MSG_STREAM urea_level: %d \n", stream->urea_level);
                pr_debug("MSG_STREAM kgh: %d \n", stream->kgh);
                pr_debug("MSG_STREAM SCR_in: %d \n", stream->SCR_in);
                pr_debug("MSG_STREAM SCR_out: %d \n", stream->SCR_out);
                pr_debug("MSG_STREAM DPF: %d \n", stream->DPF);
                pr_debug("MSG_STREAM coolant_temp: %d \n", stream->coolant_temp);
                pr_debug("MSG_STREAM tank_level: %d \n", stream->tank_level);
                pr_debug("MSG_STREAM gps_status: %d \n", stream->gps_status);
                pr_debug("MSG_STREAM longitude: %d \n", stream->longitude);
                pr_debug("MSG_STREAM latitude: %d \n", stream->latitude);
                pr_debug("MSG_STREAM mileages_total: %d \n", stream->mileages_total);
                fflush(stdout);
                break;
            case MSG_STREAM_ATT: // 0x80  补充数据流
#if debug_log
                att = (const struct shanghai_data_att *)container_of(nmsg, struct shanghai_data_att, head);
#endif
                pr_debug("MSG_STREAM_ATT Nm_mode: %d \n", att->Nm_mode);
                pr_debug("MSG_STREAM_ATT accelerator: %d \n", att->accelerator);
                pr_debug("MSG_STREAM_ATT oil_consume: %d \n", att->oil_consume);
                pr_debug("MSG_STREAM_ATT urea_tank_temp: %d \n", att->urea_tank_temp);
                pr_debug("MSG_STREAM_ATT mlh_urea_actual: %d \n", att->mlh_urea_actual);
                pr_debug("MSG_STREAM_ATT mlh_urea_total: %d \n", att->mlh_urea_total);
                pr_debug("MSG_STREAM_ATT exit_gas_temp: %d \n", att->exit_gas_temp);
                fflush(stdout);
                break;
            // 0x03-0x7F  预留
            // 0x81~0xFE  用户自定义
            default:
                break;
        }
        nmsg = nmsg->next;   // 下一个数据
    }
    fflush(stdout);
    return 0;
}
static int handle_request_userdef(const struct general_pack_shanghai* const _pack)
{
    const struct shanghai_userdef *const msg = (const struct shanghai_userdef *const)(_pack->data);
    struct upload decode;
    int len;
    len = upload_decode(&decode, msg->data, msg->_dsize);
    if(len<0)
    {
        printf("decode error!\n"); fflush(stdout);
    }
    else
    {
        printf("decode cmd : 0x%02X\n", decode.cmd); fflush(stdout);
        printf("decode total : %d\n", decode.pack_total); fflush(stdout);
        printf("decode index : %d\n", decode.pack_index); fflush(stdout);
        printf("decode checksum : 0x%02X\n", decode.checksum); fflush(stdout);
        printf("decode Model : %s\n", decode.Model); fflush(stdout);
        printf("decode len : %d\n", decode.data_len); fflush(stdout);
        printf("decode data : %s\n", decode.data); fflush(stdout);
        printf("decode CRC : 0x%02X\n", decode.CRC); fflush(stdout);
    }
    return 0;
}
static void protocol_shanghai(const struct general_pack_shanghai* const _pack)
{
    const char* ssl_type[] = {
        "0x00: NULL",
        "0x01: INFO",
        "0x02: RSA",
        "0x03: SM2",
        "“0xFE”标识异常，“0xFF”表示无效，其他预留 ",
    };
    const struct general_pack_shanghai* const pack = _pack;
    printf("0  start  : %c%c \n", pack->start[0], pack->start[1]);
    printf("2  cmd : %d \n", pack->cmd);
    printf("3  VIN : %s\n", pack->VIN);
    printf("20  soft_version : %d \n", pack->soft_version);
    printf("21  ssl[0x%02X] : %s \n", pack->ssl, ssl_type[pack->ssl&0x3]);
    printf("22  data_len : %d \n", pack->data_len);
    printf("BCC : %02X \n", pack->BCC);
    fflush(stdout);
    switch(pack->cmd)
    {
        case CMD_LOGIN:        // 车辆登入
            printf("CMD_LOGIN\n"); fflush(stdout);
            //pack_len = handle_request_login((const struct shanghai_login *const)msg_buf, pack);
            handle_request_login(pack);
            break;
        case CMD_REPORT_REAL:  // 实时信息上报
            printf("CMD_REPORT_REAL\n"); fflush(stdout);
            //handle_report_real((const struct shanghai_report_real *const)msg_buf, pack);
            handle_report_real(pack);
            //json_ShangHai_update(filename, _pack);
            break;
        case CMD_REPORT_LATER: // 补发信息上报
            printf("CMD_REPORT_LATER\n"); fflush(stdout);
            //handle_report_real((const struct shanghai_report_real *const)msg_buf, pack);
            handle_report_real(pack);
            json_ShangHai_update(filename, _pack);
            break;
        case CMD_LOGOUT:       // 车辆登出
            printf("CMD_LOGOUT\n"); fflush(stdout);
            //pack_len = handle_request_logout((const struct shanghai_logout *const)msg_buf, pack);
            handle_request_logout(pack);
            break;
        case CMD_UTC:          // 终端校时
            printf("CMD_UTC\n"); fflush(stdout);
            json_ShangHai_update(filename, _pack);
            break;
        case CMD_USERDEF:      // 用户自定义
            printf("CMD_USERDEF\n"); fflush(stdout);
            handle_request_userdef(pack);
            json_ShangHai_update(filename, _pack);
            break;
        default:
            printf("default\n"); fflush(stdout);
            json_ShangHai_update(filename, _pack);
            break;
    }
}
static void decode_test(struct obd_agree_obj* const _obd_fops, const uint8_t pack[], const uint16_t _psize, void* const _msg_buf, const uint16_t _msize)
{
    int len = 0;
    len =_obd_fops->fops->decode(_obd_fops, pack, _psize, _msg_buf, _msize);
    printf("decode_test len : %d \n", len); fflush(stdout);
    if(len<0) return;
    switch (_obd_fops->fops->protocol) // switch (_agree_ofp->protocol())
    {
        case PRO_TYPE_CCU:    // CCU
            //
            break;
        case PRO_TYPE_SHH:    // ShangHai
            printf("Protocol ShangHai \n"); fflush(stdout);
            protocol_shanghai((const struct general_pack_shanghai* const)_msg_buf);
            break;
        default:
            break;
    }
}

static const struct agreement_ofp* _agree_obd=NULL;
static uint8_t obd_buf[1024*10];
static uint8_t msg_buf[4096];
//void agreement_init(void)
//{
//    _agree_obd = create_agree_obd_shanghai();
//    _agree_obd->init(0, (const uint8_t*)"IMEI1234567890ABCDEF", 2, "INFO");
//}
void agreement_test(void)
{
    int len = 0;
    time_t timer = time(NULL);
//    DateTime      utctime   = {.year = 1970, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 0};
//    DateTime      localtime = {.year = 1970, .month = 1, .day = 1, .hour = 8, .minute = 0, .second = 0};

    char _obd_obj_buf[sizeof(struct obd_agree_obj)];
    struct obd_agree_obj* const _obd_obj = obd_agree_obj_shanghai.fops->constructed(&obd_agree_obj_shanghai, _obd_obj_buf);
    //_agree_obd = create_agree_obd_shanghai();
    //_agree_obd->init(0, (const uint8_t*)"IMEI1234567890ABCDEF", 2, "INFO");
    _obd_obj->fops->init(_obd_obj, 0, (const uint8_t*)"IMEI1234567890ABCDEF", (const uint8_t*)"VIN0123456789ABCDEF", 2, "RSA");

//    memset(filename, 0, sizeof (filename));
//    if(timer > INT32_MAX)
//    {
//      utctime = GregorianCalendarDateAddSecond(utctime, INT32_MAX);
//      utctime = GregorianCalendarDateAddSecond(utctime, (int)(timer - INT32_MAX));
//    }
//    else
//    {
//      utctime = GregorianCalendarDateAddSecond(utctime, (int)timer);
//    }

//    GregorianCalendarDateToModifiedJulianDate(utctime);
//    localtime = GregorianCalendarDateAddHour(utctime, 8);
//    snprintf(filename, sizeof (filename)-1, "%s-%d%d%d-%d:%d:%d.json", "VIN1234567890", localtime.year, localtime.month, localtime.day, localtime.hour, localtime.minute, localtime.second);
//    json_table_update(filename, "ShangHai");

#if 1
    len = _obd_obj->fops->base->pack.login(_obd_obj, timer, 0, (const uint8_t*)"VIN1234567890ABCDEF1234567890",  (const uint8_t*)"VIN1234567890ABCDEF", "ATT", obd_buf, sizeof (obd_buf));
    printf("login pack len : %d\n", len);
    fflush(stdout);
    decode_test(_agree_obd, obd_buf, (const uint16_t)len, msg_buf, sizeof (msg_buf));
    fflush(stdout);
#endif
#if 0
    time(&timer);
    len = _agree_obd->logout(timer, 0, obd_buf, sizeof (obd_buf));
    printf("logout pack len : %d\n", len);
    fflush(stdout);
    decode_test(_agree_obd, obd_buf, (const uint16_t)len, msg_buf, sizeof (msg_buf));
    fflush(stdout);
#endif
#if 0
    time(&timer);
    len = _agree_obd->utc(obd_buf, sizeof (obd_buf));
    printf("UTC pack len : %d\n", len);
    fflush(stdout);
    decode_test(_agree_obd, obd_buf, (const uint16_t)len, msg_buf, sizeof (msg_buf));
    fflush(stdout);
#endif
#if 1  // 自定义
    time(&timer);
    uint8_t buffer[sizeof(struct upload)];
    struct upload* _load = upload_init(UPLOAD_LOGIN, 0, 0, 0x12345678, "OBD1234567890ABCDEF", "Hello", 5);
    memset(buffer, 0, sizeof (buffer));
    len = upload_encode(_load, buffer, sizeof (buffer));
    len = _obd_obj->fops->base->pack.encode(_obd_obj, buffer, len, obd_buf, sizeof (obd_buf));
    printf("encode pack len : %d\n", len);
    fflush(stdout);
    decode_test(_agree_obd, obd_buf, (const uint16_t)len, msg_buf, sizeof (msg_buf));
    fflush(stdout);
#endif
#if 0
    struct report_gps gps;
    OBD_DATA obd;
    memset(&gps, 0, sizeof (gps));
    gps.status = 0x3<<1;
    gps.speed = 30;
    gps.longitude = 30;
    gps.latitude = 40;
    memset(&obd, 0, sizeof (obd));
    memcpy(obd.ECUInfo.VIN, "VIN1234567890ABCDEFGHIJ", 19);
    obd.RTData.Velocity = 100;
    obd.RTData.EngineRev = 200;
    obd.RTData.Mileage = 300;
    obd.RTData.FuelConsumption = 400;
    obd.DTCInfo.Current = 3;
    obd.DTCInfo.CurrentDTC[0] = '0';
    obd.DTCInfo.CurrentDTC[1] = '1';
    obd.DTCInfo.CurrentDTC[2] = '2';
    obd.DTCInfo.CurrentDTC[3] = '3';
    obd.DTCInfo.CurrentDTC[4] = '4';
    obd.DTCInfo.CurrentDTC[5] = '5';
    time(&timer);
    len = _agree_obd->report(shanghai_report_msg(timer, &obd, &gps, msg_buf, sizeof (msg_buf)), obd_buf, sizeof (obd_buf));
    printf("report pack len : %d\n", len);
    fflush(stdout);
    memset(&msg_buf, 0, sizeof (msg_buf));
    //_agree_obd->decode(obd_buf, (const uint16_t)len, msg_buf, sizeof (msg_buf));
    decode_test(_agree_obd, obd_buf, (const uint16_t)len, msg_buf, sizeof (msg_buf));
    fflush(stdout);
    //utc_main();
    fflush(stdout);
#endif
}





