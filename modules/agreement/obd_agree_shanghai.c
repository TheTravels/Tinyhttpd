/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : obd_agree_shanghai.c
* Author             : Merafour
* Last Modified Date : 11/12/2019
* Description        : 《上海市车载排放诊断（OBD）系统在线接入技术指南（试行）发布稿.pdf》.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#include "obd_agree_shanghai.h"
#include "encrypt.h"
#include <string.h>
//#include <memory.h>
#include <pthread.h>
#include <malloc.h>
#include "upload.h"
#include "obd_agree_fops.h"
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

//#include "cJSON/cJSON.h"
//#include "cJSON/cJSON.h"

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
// 需要 GNU 扩展支持, -std=gnu++11
#define container_of(ptr, type, member) ({   const typeof(((type *)0)->member)*__mptr = (ptr);    (type *)((char *)__mptr - offsetof(type, member)); })
//#define container_of(ptr, type, member) (0)

//obd_agree_shanghai_ptr obd_ptr_ = NULL;

/*obd_agree_shanghai_ptr &get_instance()
{
    if (NULL == obd_ptr_)
        obd_ptr_.reset(new obd_agree_shanghai);
    return obd_ptr_;
}*/

static uint16_t count=0;
//static uint8_t report_msg_buf[4096];
struct shanghai_report_real* shanghai_report_msg(const uint32_t UTC, const OBD_DATA *_obd, const struct report_gps* const _gps, uint8_t _msg_buf[], const uint16_t _msg_size)
{
    struct shanghai_report_real *msg = (struct shanghai_report_real *)_msg_buf;
    uint16_t index=0;
    uint8_t len=0;
    struct report_head* nmsg=NULL;  // next msg
    struct report_head* fault=NULL;
    struct shanghai_data_obd *obd=NULL;
    struct shanghai_data_stream *stream=NULL;
    struct shanghai_data_att *att=NULL;
    memset(_msg_buf, 0, _msg_size);
    msg->msg = NULL;
    msg->count = count++;
    UTC2hhmmss(UTC, msg->UTC, sizeof (msg->UTC));
    index = sizeof (struct shanghai_report_real);
    // OBD
    obd = (struct shanghai_data_obd *)&_msg_buf[index];
    index += sizeof (struct shanghai_data_obd);
    obd->protocol = 2;     // J1939
    obd->MIL      = (uint8_t)_obd->SystemCheckState.MIL;
    memcpy(obd->VIN, _obd->ECUInfo.VIN, 17);     // 车辆识别码（VIN） 17
    memcpy(obd->SVIN, _obd->ECUInfo.VIN, 18);    // 软件标定识别号 18
    memcpy(obd->CVN, _obd->ECUInfo.VIN, 20);     // 标定验证码（CVN） 18
    memcpy(obd->IUPR, _obd->ECUInfo.VIN, 20);    // IUPR 值  36
//    printf("obd->VIN: %s ECUInfo.VIN:%s\n", obd->VIN, _obd->ECUInfo.VIN); fflush(stdout);
//    printf("obd->SVIN: %s \n", obd->SVIN); fflush(stdout);
//    printf("obd->CVN: %s \n", obd->SVIN); fflush(stdout);
    obd->fault_total = (uint8_t)_obd->DTCInfo.Current;
    // 填充故障码
    fault = &obd->fault_list;
    for(len=0; len<obd->fault_total; len++)
    {
        //printf("shanghai_report_msg[%d]: %d \n", obd->fault_total, len); fflush(stdout);
        fault->data = _obd->DTCInfo.CurrentDTC[len];
        fault->next = NULL;
        if(len>=obd->fault_total) break;
        fault->next = (struct report_head*)&_msg_buf[index];
        fault = fault->next;
        index += sizeof (struct report_head);
    }
    msg->msg = &obd->head;
    nmsg = msg->msg;
    nmsg->type_msg = MSG_OBD;
    pr_debug("type_msg : %d \n", nmsg->type_msg); //fflush(stdout);
    // stream
    stream = (struct shanghai_data_stream *)&_msg_buf[index];
    index += sizeof (struct shanghai_data_stream);
    stream->speed = _obd->RTData.Velocity;
    stream->kPa=1;            // 2  大气压力  BYTE  kPa 数据长度：1btyes 精度：0.5/bit 偏移量：0 数据范围：0~125kPa “0xFF”表示无效
    stream->Nm=1;            // 3  发动机净输出扭矩（实际扭矩百分比）  BYTE  %  数据长度：1btyes精度：1%/bit 偏移量：-125 数据范围：-125~125% “0xFF”表示无效
    stream->Nmf=1;            // 4 摩擦扭矩（摩擦扭矩百分比） BYTE  % 数据长度：1btyes 精度：1%/bit 偏移量：-125 数据范围：-1250~125% “0xFF”表示无效
    stream->rpm=_obd->RTData.EngineRev;            // 5  发动机转速  WORD  rpm 数据长度：2btyes 精度：0.125rpm/bit 偏移量：0 数据范围：0~8031.875rpm “0xFF,0xFF”表示无效
    stream->Lh=1;            // 7  发动机燃料流量  WORD  L/h 数据长度：2btyes 精度：0.05L/h 偏移量：0 数据范围：0~3212.75L/h “0xFF,0xFF”表示无效
    stream->ppm_up=1;         // 9 SCR 上游 NOx 传感器输出值（后处理上游氮氧浓度） WORD  ppm 数据长度：2btyes 精度：0.05ppm/bit 偏移量：-200 数据范围：-200~3212.75ppm “0xFF,0xFF”表示无效
    stream->ppm_down=1;       // 11 SCR 下游 NOx 传感器输出值（后处理下游氮氧浓度） WORD  ppm 数据长度：2btyes 精度：0.05ppm/bit 偏移量：-200 数据范围：-200~3212.75ppm  “0xFF,0xFF”表示无效
    stream->urea_level=1;      // 13 反应剂余量 （尿素箱液位） BYTE  % 数据长度：1btyes 精度：0.4%/bit 偏移量：0 数据范围：0~100% “0xFF”表示无效
    stream->kgh=1;            // 14  进气量  WORD  kg/h 数据长度：2btyes 精度：0.05kg/h per bit 偏移量：0 数据范围：0~3212.75ppm “0xFF,0xFF”表示无效
    stream->SCR_in=1;         // 16 SCR 入口温度（后处理上游排气温度） WORD  ℃ 数据长度：2btyes 精度：0.03125 ℃ per bit 偏移量：-273 数据范围：-273~1734.96875℃ “0xFF,0xFF”表示无效
    stream->SCR_out=1;        // 18 SCR 出口温度（后处理下游排气温度） WORD  ℃ 数据长度：2btyes 精度：0.03125 ℃ per bit 偏移量：-273 数据范围：-273~1734.96875℃ “0xFF,0xFF”表示无效
    stream->DPF=1;            // 20 DPF 压差（或 DPF排气背压） WORD  kPa 数据长度：2btyes 精度：0.1 kPa per bit 偏移量：0 数据范围：0~6425.5 kPa “0xFF,0xFF”表示无效
    stream->coolant_temp=1;    // 22  发动机冷却液温度  BYTE  ℃ 数据长度：1btyes 精度：1 ℃/bit 偏移量：-40 数据范围：-40~210℃ “0xFF”表示无效
    stream->tank_level=1;      // 23  油箱液位  BYTE  % 数据长度：1btyes 精度：0.4% /bit 偏移量：0 数据范围：0~100% “0xFF”表示无效
    stream->gps_status=_gps->status;      // 24  定位状态  BYTE    数据长度：1btyes
    stream->longitude=_gps->longitude;      // 25  经度  DWORD   数据长度：4btyes 精度：0.000001° per bit 偏移量：0 数据范围：0~180.000000° “0xFF,0xFF,0xFF,0xFF”表示无效
    stream->latitude=_gps->latitude;       // 29  纬度  DWORD   数据长度：4btyes 精度：0.000001 度  per bit 偏移量：0 数据范围：0~180.000000° “0xFF,0xFF,0xFF,0xFF”表示无效
    stream->mileages_total=_obd->RTData.Mileage; // 33 累计里程 （总行驶里程） DWORD  km 数据长度：4btyes 精度：0.1km per bit 偏移量：0 “0xFF,0xFF,0xFF,0xFF”表示无效

    nmsg->next = &stream->head;
    nmsg = nmsg->next;
    nmsg->type_msg = MSG_STREAM;
    pr_debug("type_msg : %d \n", nmsg->type_msg); //fflush(stdout);
    // att
    att = (struct shanghai_data_att *)&_msg_buf[index];
    index += sizeof (struct shanghai_data_att);
    att->Nm_mode=1;           // 0  发动机扭矩模式  BYTE    0：超速失效 1：转速控制 2：扭矩控制 3：转速/扭矩控制 9：正常
    att->accelerator=1;      // 1  油门踏板  BYTE  % 数据长度：1btyes 精度：0.4%/bit 偏移量：0 数据范围：0~100% “0xFF”表示无效
    att->oil_consume=_obd->RTData.FuelConsumption;      // 2 累计油耗 （总油耗） DWORD  L 数据长度：4btyes 精度：0.5L per bit 偏移量：0 数据范围：0~2 105 540 607.5L “0xFF,0xFF,0xFF,0xFF”表示无效
    att->urea_tank_temp=1;    // 6  尿素箱温度  BYTE  ℃ 数据长度：1btyes 精度：1 ℃/bit 偏移量：-40 数据范围：-40~210℃ “0xFF”表示无效
    att->mlh_urea_actual=1;  // 7  实际尿素喷射量  DWORD  ml/h 数据长度：4btyes 精度：0.01 ml/h per bit 偏移量：0 数据范围：0 “0xFF,0xFF,0xFF,0xFF”表示无效
    att->mlh_urea_total=1;   // 11 累计尿素消耗 （总尿素消耗） DWORD  g 数据长度：4btyes 精度：1 g per bit 偏移量：0 数据范围：0  “0xFF,0xFF,0xFF,0xFF”表示无效
    att->exit_gas_temp=1;    // 15  DPF 排气温度  WORD  ℃ 数据长度：2btyes 精度：0.03125 ℃ per bit 偏移量：-273 数据范围：-273~1734.96875℃ “0xFF,0xFF”表示无效
    // ;
    nmsg->next = &att->head;
    nmsg = nmsg->next;
    nmsg->type_msg = MSG_STREAM_ATT;
    pr_debug("type_msg : %d \n", nmsg->type_msg); //fflush(stdout);
    nmsg->next=NULL;
    return msg;
}

// cmd
static enum cmd_unit_shanghai get_cmd(const enum general_pack_type _pack_type)
{
    enum cmd_unit_shanghai _cmd = CMD_NULL;
    switch(_pack_type)
    {
        case GEN_PACK_LOGIN:        // 车辆登入
            _cmd = CMD_LOGIN;
            break;
        case GEN_PACK_REPORT_REAL:  // 实时信息上报
            _cmd = CMD_REPORT_REAL;
            break;
        case GEN_PACK_REPORT_LATER: // 补发信息上报
            _cmd = CMD_REPORT_LATER;
            break;
        case GEN_PACK_LOGOUT:       // 车辆登出
            _cmd = CMD_LOGOUT;
            break;
        case GEN_PACK_UTC:          // 终端校时
            _cmd = CMD_UTC;
            break;
        case GEN_PACK_USERDEF:      // 用户自定义
            _cmd = CMD_USERDEF;
            break;
        default:
            _cmd = CMD_NULL;
            break;
    }
    return _cmd;
}

// 初始化函数
//static struct general_pack_shanghai general_pack_en;
static void init(struct obd_agree_obj* const _obd_fops, const uint16_t count, const uint8_t IMEI[], const uint8_t VIN[], const uint16_t soft_version, const char *ssl)
{
    const struct {
        uint8_t value;
        char ssl[7];
    }ssl_list[] = {
        {0x01, "INFO"},  // 0x01：数据不加密
        {0x02, "RSA" },  // 0x02：数据经过 RSA 算法加密
        {0x03, "SM2" },  // 0x03：数据经过国密 SM2 算法加密
        {0xFF, "NULL"},  // “0xFE”标识异常，“0xFF”表示无效，其他预留
    };
    uint8_t index=0;
    struct general_pack_shanghai* const pack = &_obd_fops->_gen_pack;
    (void)count;
    (void)IMEI;
    //memset(&login_pack, 0, sizeof (login_pack));
    memset(pack, 0, sizeof (struct general_pack_shanghai));
    // 0  起始符  STRING  固定为 ASCII 字符’##’，用“0x23，0x23”表示
    pack->start[0] = '#';
    pack->start[1] = '#';
    // default
    pack->ssl = 0x01;                       // 0x01：数据不加密
    for(index=0; index<(sizeof (ssl_list)/sizeof (ssl_list[0])); index++)
    {
        if(0==strcmp(ssl, ssl_list[index].ssl))
        {
            pack->ssl = ssl_list[index].value;
            break;
        }
    }
    pack->soft_version = soft_version&0xFF; // 软件版本
    //memcpy(pack->VIN, "VIN0123456789ABCDEF", sizeof (pack->VIN)-1);
    memcpy(pack->VIN, VIN, sizeof (pack->VIN)-1);
    //memcpy(&general_pack_de, pack, sizeof (general_pack_de));
    _obd_fops->_print = 0,
    _obd_fops->_relay = 0,
    rsa_int();
}
/**
 * 通用校验包函数
 */
static int check_pack_general(struct obd_agree_obj* const _obd_fops, const void* const _data, const uint16_t _dsize)
{
    uint8_t bcc=0;
    uint16_t index=0;
    struct general_pack_shanghai _general_pack;
    struct general_pack_shanghai *const _pack = &_general_pack;
    const uint8_t* const data = (const uint8_t*)_data;
    index=0;
    // 0  起始符  STRING  固定为 ASCII 字符’##’，用“0x23，0x23”表示
    _pack->start[0] = data[index++];
    _pack->start[1] = data[index++];
    if(('#'!=_pack->start[0]) || ('#'!=_pack->start[1])) return ERR_DECODE_PACKS; // 包头错误
    _pack->cmd = data[index++];                                // 2  命令单元  BYTE  命令单元定义见  表 A2  命令单元定义
    memcpy(_pack->VIN, &data[index], 17);                      // 3  车辆识别号  STRING 车辆识别码是识别的唯一标识，由 17 位字码组成，字码应符合 GB16735 中 4.5 的规定
    index += 17;
    _pack->soft_version = data[index++];                       // 20  终端软件版本号  BYTE  终端软件版本号有效值范围 0~255
    _pack->ssl = data[index++];                                // 21  数据加密方式  BYTE
    _pack->data_len = merge_16bit(data[index], data[index+1]); // 22  数据单元长度  WORD 数据单元长度是数据单元的总字节数，有效范围：0~65531
    index += 2;
    //pr_debug("decode_pack_general _dsize:%d | %d \n\n", _dsize, (_pack->data_len+index+1)); fflush(stdout);
    if(_dsize<(_pack->data_len+index+1)) return  ERR_DECODE_PACKL; // 包长度不够
    //pr_debug("decode_pack_general switch %d \n", index); fflush(stdout);
    // 倒数第 1  校验码  BYTE 采用 BCC（异或校验）法，校验范围聪明星单元的第一个字节开始，同后一个字节异或，直到校验码前一字节为止，校验码占用一个字节
    //bcc = BCC_check_code(&data[2], index+_pack->data_len-1);
    bcc = BCC_check_code(&data[2], index+_pack->data_len-2);  // start: 2 byte
    index += _pack->data_len;
    _pack->BCC = data[index++]; // data[index+_pack->data_len];
    if(bcc != _pack->BCC) return ERR_DECODE_PACKBCC;                // BCC 校验错误
    return  index;
}
// 解码包函数
static int handle_decode(struct obd_agree_obj* const _obd_fops, const uint8_t _pack[], const uint16_t _psize, void * const _msg_buf, const uint16_t _msize)
{
#if 0//debug_log
    const char* ssl_type[] = {
        "0x00: NULL"
        "0x01: INFO",
        "0x02: RSA",
        "0x03: SM2",
        "“0xFE”标识异常，“0xFF”表示无效，其他预留 ",
    };
#endif
    //memset(&general_pack_de, 0, sizeof (general_pack_de));
    //struct general_pack_shanghai *const pack = &general_pack_de;
    //struct general_pack_shanghai *const pack = (struct general_pack_shanghai *const)_msg_buf;
    struct general_pack_shanghai *const pack = &_obd_fops->_gen_pack;
    int head_len = sizeof (struct general_pack_shanghai);
    int pack_len = 0;
    //uint8_t msg_buf[1024];
    //uint8_t ack[1024];
    //printf("@%s-%d \n", __func__, __LINE__);
    memset(pack, 0, sizeof (struct general_pack_shanghai));
    pack->data = ((char*)_msg_buf)+head_len;//&_msg_buf[head_len];
    //pack->data = &msg_buf[0];
    //printf("@%s-%d \n", __func__, __LINE__);
    pack_len = _obd_fops->fops->decode_pack_general(_obd_fops, _pack, _psize, pack->data, _msize-head_len);
    //pack_len = decode_pack_general(_pack, _psize, pack, pack->data, 1024);
    //pack_len = decode_pack_general(_pack, _psize, pack, msg_buf, sizeof (msg_buf));
    //printf("@%s-%d \n", __func__, __LINE__);
#if 0
    pr_debug("handle_decode pack len:%d \n\n", pack_len);
    pr_debug("0  start  : %c%c \n", pack->start[0], pack->start[1]);
    pr_debug("2  cmd : %d \n", pack->cmd);
    pr_debug("3  VIN : %s\n", pack->VIN);
    pr_debug("20  soft_version : %d \n", pack->soft_version);
    pr_debug("21  ssl : %s \n", ssl_type[pack->ssl&0x3]);
    pr_debug("22  data_len : %d \n", pack->data_len);
    pr_debug("BCC : %02X \n", pack->BCC);
    switch(pack->cmd)
    {
        case CMD_LOGIN:        // 车辆登入
            pr_debug("CMD_LOGIN\n"); //fflush(stdout);
            //pack_len = handle_request_login((const struct shanghai_login *const)msg_buf, pack);
            pack_len = handle_request_login((const struct shanghai_login *const)pack->data, pack);
            break;
        case CMD_REPORT_REAL:  // 实时信息上报
            pr_debug("CMD_REPORT_REAL\n"); //fflush(stdout);
            //handle_report_real((const struct shanghai_report_real *const)msg_buf, pack);
            pack_len = handle_report_real((const struct shanghai_report_real *const)pack->data, pack);
            break;
        case CMD_REPORT_LATER: // 补发信息上报
            pr_debug("CMD_REPORT_LATER\n"); //fflush(stdout);
            //handle_report_real((const struct shanghai_report_real *const)msg_buf, pack);
            pack_len = handle_report_real((const struct shanghai_report_real *const)pack->data, pack);
            break;
        case CMD_LOGOUT:       // 车辆登出
            pr_debug("CMD_LOGOUT\n"); //fflush(stdout);
            //pack_len = handle_request_logout((const struct shanghai_logout *const)msg_buf, pack);
            pack_len = handle_request_logout((const struct shanghai_logout *const)pack->data, pack);
            break;
        case CMD_UTC:          // 终端校时
            pr_debug("CMD_UTC\n"); //fflush(stdout);
            pack_len = 0;
            break;
        case CMD_USERDEF:      // 用户自定义
            pr_debug("CMD_USERDEF\n"); //fflush(stdout);
            pack_len = 0;
            break;
        default:
            pr_debug("default\n"); //fflush(stdout);
            pack_len = 0;
            break;
    }
#endif
    return  pack_len;
}

// 自定义编码
static int userdef_encode(struct obd_agree_obj* const _obd_fops, const void * const __udef, void * const _buffer, const uint16_t _size)
{
    return upload_encode((const struct upload *)__udef, _buffer, _size);
}
// 自定义解码
static int userdef_decode(struct obd_agree_obj* const _obd_fops, void * const __udef, const void * const _data, const uint16_t _size)
{
    return upload_decode((struct upload *)__udef, _data, _size);
}
// 更新推送
static int upload_push(struct obd_agree_obj* const _obd_fops, const uint32_t checksum_cfg, const uint32_t checksum_fw, void * const _buffer, const uint16_t _size)
{
    struct upload* _load = NULL;
    uint8_t buffer[sizeof(struct upload)];
    int len = 0;
    (void)checksum_cfg;
    _load = upload_init(UPLOAD_PUSH, 0, 10*1024, checksum_fw, "OBD-4G", "000000000000", 12);
    len = upload_encode(_load, buffer, sizeof(buffer));
    return _obd_fops->fops->base->pack.encode(_obd_fops, buffer, len, (uint8_t*)_buffer, _size);
}

static int encode_msg_login(const struct shanghai_login *const msg, uint8_t buf[], const uint16_t _size)
{
    uint16_t index=0;
    uint16_t data_len=0;
    data_len = sizeof (msg->UTC)-1 + sizeof (msg->count) + sizeof (msg->ICCID)-1;
    if(data_len>_size) return ERR_ENCODE_PACKL;
    //memset(buf, 0, _size);
    index=0;
    memcpy(&buf[index], msg->UTC, sizeof (msg->UTC)-1);     // 0  数据采集时间  BYTE[6]  时间定义见 A4.4
    BUILD_BUG_ON(sizeof (msg->UTC)-1 != 6);
    index += 6;
    index += bigw_16bit(&buf[index], msg->count);           // 6  登入流水号  WORD 车载终端每登入一次，登入流水号自动加 1，从 1开始循环累加，最大值为 65531，循环周期为天。
    memcpy(&buf[index], msg->ICCID, sizeof (msg->ICCID)-1); // 10  SIM 卡号  STRING SIM 卡 ICCID 号（ICCID 应为终端从 SIM 卡获取的值，不应人为填写或修改）。
    BUILD_BUG_ON(sizeof (msg->ICCID)-1 != 20);                                               // 倒数第 1  String     1 终止符,固定 ASCII 码“~”，用 0x7E 表示
    index += 20;
    return index; // len
}
static int encode_msg_logout(const struct shanghai_logout *const msg, uint8_t buf[], const uint16_t _size)
{
    uint16_t index=0;
    uint16_t data_len=0;
    data_len = sizeof (msg->UTC)-1 + sizeof (msg->count);
    if(data_len>_size) return ERR_ENCODE_PACKL;
    //memset(buf, 0, _size);
    index=0;
    memcpy(&buf[index], msg->UTC, sizeof (msg->UTC)-1);     // 0  数据采集时间  BYTE[6]  时间定义见 A4.4
    BUILD_BUG_ON(sizeof (msg->UTC)-1 != 6);
    index += sizeof (msg->UTC)-1;
    index += bigw_16bit(&buf[index], msg->count);           // 6  登入流水号  WORD 车载终端每登入一次，登入流水号自动加 1，从 1开始循环累加，最大值为 65531，循环周期为天。
    return index; // len
}


// A4.5.2  实时信息上报
static int encode_msg_report_real(const struct shanghai_report_real *const msg, uint8_t buf[], const uint16_t _size)
{
    uint16_t index=0;
    uint16_t data_len=0;
    uint8_t i=0;
    struct report_head* nmsg=NULL;  // next msg
    const struct report_head* fault=NULL;
    const struct shanghai_data_obd *obd=NULL;
    const struct shanghai_data_stream *stream=NULL;
    const struct shanghai_data_att *att=NULL;
    uint8_t fault_count=0;
    data_len = sizeof (struct shanghai_report_real) + sizeof (struct report_head);
    if(data_len>_size) return ERR_ENCODE_PACKL;
    //memset(buf, 0, _size);
    index=0;
    memcpy(&buf[index], msg->UTC, sizeof (msg->UTC)-1);     // 数据采集时间  6  BYTE[6]  时间定义见 A4.4
    BUILD_BUG_ON(sizeof (msg->UTC)-1 != 6);
    index += 6;
    index += bigw_16bit(&buf[index], msg->count);           // 信息流水号  2  WORD
    // 消息，可包含多条
    //if(NULL==msg->msg) return index; // 无消息
    nmsg = msg->msg;
    //pr_debug("msg.msg : %d \n", (NULL!=msg->msg)); fflush(stdout);
    while(NULL!=nmsg)
    {
        //pr_debug("type_msg : %d \n", nmsg->type_msg); fflush(stdout);
        switch (nmsg->type_msg)
        {
            // 1） OBD 信息数据格式和定义见表 A.6 所示。
            case MSG_OBD:       // 0x01  OBD 信息
                if((index+sizeof (struct shanghai_data_obd))>_size) return ERR_ENCODE_PACKL;
                pr_debug("MSG_OBD : %d \n", index); //fflush(stdout);
                obd = (const struct shanghai_data_obd *) container_of(nmsg, struct shanghai_data_obd, head);
                buf[index++] = nmsg->type_msg;
                buf[index++] = obd->protocol;  // OBD 诊断协议  1  BYTE 有效范围 0~2，“0”代表 IOS15765，“1”代表IOS27145，“2”代表 SAEJ1939，“0xFE”表示无效。
                buf[index++] = obd->MIL;       // MIL 状态  1  BYTE 有效范围 0~1，“0”代表未点亮，“1”代表点亮。“0xFE”表示无效。
                index += bigw_16bit(&buf[index], obd->status); // 诊断支持状态  2  WORD
                index += bigw_16bit(&buf[index], obd->ready); // 诊断就绪状态  2  WORD
                // 车辆识别码（VIN） 17  STRING
                memcpy(&buf[index], obd->VIN, sizeof (obd->VIN)-1);
                BUILD_BUG_ON(sizeof (obd->VIN)-1 != 17);
                index += 17;
                // 软件标定识别号 18  STRING 软件标定识别号由生产企业自定义，字母或数字组成，不足后面补字符“0”。
                memcpy(&buf[index], obd->SVIN, sizeof (obd->SVIN)-1);
                BUILD_BUG_ON(sizeof (obd->SVIN)-1 != 18);
                index += 18;
                // 标定验证码（CVN） 18  STRING 标定验证码由生产企业自定义，字母或数字组成，不足后面补字符“0”。
                memcpy(&buf[index], obd->CVN, sizeof (obd->CVN)-1);
                BUILD_BUG_ON(sizeof (obd->CVN)-1 != 18);
//                printf("obd->VIN: %s \n", obd->VIN); fflush(stdout);
//                printf("obd->SVIN: %s \n", obd->SVIN); fflush(stdout);
//                printf("obd->CVN: %s \n", obd->SVIN); fflush(stdout);
                index += 18;
                // IUPR 值  36  DSTRING  定义参考 SAE J 1979-DA 表 G11.
                //memcpy(&buf[index], obd->IUPR, sizeof (obd->IUPR)-1);
                for(i=0; i<18; i++)
                {
                    index += bigw_16bit(&buf[index], obd->IUPR[i]);
                }
                BUILD_BUG_ON(sizeof (obd->IUPR) != 36);
                //index += 36;
                buf[index++] = obd->fault_total;// 故障码总数  1  BYTE  有效值范围：0~253，“0xFE”表示无效。
                // 故障码信息列表 ∑每个故障码信息长度 N*BYTE（4） 每个故障码为四字节，可按故障实际顺序进行排序。
                fault = &(obd->fault_list);
                //pr_debug("MSG_OBD ... index:%d fault_total:%d \n", index, obd->fault_total); fflush(stdout);
                fault_count=0;
                //while(NULL!=fault)
                //while((fault_count<obd->fault_total) && (NULL!=fault))
                for(fault_count=0; fault_count<obd->fault_total; fault_count++)
                {
                    //printf("encode_msg_report_real[%d]: %d %c \n", obd->fault_total, fault_count, fault->data); fflush(stdout);
                    index += bigw_32bit(&buf[index], fault->data);  // 故障码, BYTE（4）
                    fault = fault->next;  // 下一个数据
                    if(NULL==fault) break;
                }
                //pr_debug("MSG_OBD ... index:%d \n", index); fflush(stdout);
                break;
            // 2）数据流信息数据格式和定义见表 A.7 所示，补充数据流信息数据格式和定义见表 A.8 所示。
            case MSG_STREAM:     // 0x02  数据流信息
                if((index+sizeof (struct shanghai_data_stream))>_size) return ERR_ENCODE_PACKL;
                pr_debug("MSG_STREAM : %d \n", index); //fflush(stdout);
                stream = (const struct shanghai_data_stream *)container_of(nmsg, struct shanghai_data_stream, head);
                buf[index++] = nmsg->type_msg;
                index += bigw_16bit(&buf[index], stream->speed);           // 0  车速  WORD  km/h 数据长度：2btyes 精度：1/256km/h/ bit 偏移量：0 数据范围：0~250.996km/h “0xFF,0xFF”表示无效
                buf[index++] = stream->kPa;                                // 2  大气压力  BYTE  kPa 数据长度：1btyes 精度：0.5/bit 偏移量：0 数据范围：0~125kPa “0xFF”表示无效
                buf[index++] = stream->Nm;                                 // 3  发动机净输出扭矩（实际扭矩百分比）  BYTE  %  数据长度：1btyes精度：1%/bit 偏移量：-125 数据范围：-125~125% “0xFF”表示无效
                buf[index++] = stream->Nmf;                                // 4 摩擦扭矩（摩擦扭矩百分比） BYTE  % 数据长度：1btyes 精度：1%/bit 偏移量：-125 数据范围：-1250~125% “0xFF”表示无效
                index += bigw_16bit(&buf[index], stream->rpm);             // 5  发动机转速  WORD  rpm 数据长度：2btyes 精度：0.125rpm/bit 偏移量：0 数据范围：0~8031.875rpm “0xFF,0xFF”表示无效
                index += bigw_16bit(&buf[index], stream->Lh);              // 7  发动机燃料流量  WORD  L/h 数据长度：2btyes 精度：0.05L/h 偏移量：0 数据范围：0~3212.75L/h “0xFF,0xFF”表示无效
                index += bigw_16bit(&buf[index], stream->ppm_up);          // 9 SCR 上游 NOx 传感器输出值（后处理上游氮氧浓度） WORD  ppm 数据长度：2btyes 精度：0.05ppm/bit 偏移量：-200 数据范围：-200~3212.75ppm “0xFF,0xFF”表示无效
                index += bigw_16bit(&buf[index], stream->ppm_down);        // 11 SCR 下游 NOx 传感器输出值（后处理下游氮氧浓度） WORD  ppm 数据长度：2btyes 精度：0.05ppm/bit 偏移量：-200 数据范围：-200~3212.75ppm  “0xFF,0xFF”表示无效
                buf[index++] = stream->urea_level;                         // 13 反应剂余量 （尿素箱液位） BYTE  % 数据长度：1btyes 精度：0.4%/bit 偏移量：0 数据范围：0~100% “0xFF”表示无效
                index += bigw_16bit(&buf[index], stream->kgh);             // 14  进气量  WORD  kg/h 数据长度：2btyes 精度：0.05kg/h per bit 偏移量：0 数据范围：0~3212.75ppm “0xFF,0xFF”表示无效
                index += bigw_16bit(&buf[index], stream->SCR_in);          // 16 SCR 入口温度（后处理上游排气温度） WORD  ℃ 数据长度：2btyes 精度：0.03125 ℃ per bit 偏移量：-273 数据范围：-273~1734.96875℃ “0xFF,0xFF”表示无效
                index += bigw_16bit(&buf[index], stream->SCR_out);         // 18 SCR 出口温度（后处理下游排气温度） WORD  ℃ 数据长度：2btyes 精度：0.03125 ℃ per bit 偏移量：-273 数据范围：-273~1734.96875℃ “0xFF,0xFF”表示无效
                index += bigw_16bit(&buf[index], stream->DPF);             // 20 DPF 压差（或 DPF排气背压） WORD  kPa 数据长度：2btyes 精度：0.1 kPa per bit 偏移量：0 数据范围：0~6425.5 kPa “0xFF,0xFF”表示无效
                buf[index++] = stream->coolant_temp;                       // 22  发动机冷却液温度  BYTE  ℃ 数据长度：1btyes 精度：1 ℃/bit 偏移量：-40 数据范围：-40~210℃ “0xFF”表示无效
                buf[index++] = stream->tank_level;                         // 23  油箱液位  BYTE  % 数据长度：1btyes 精度：0.4% /bit 偏移量：0 数据范围：0~100% “0xFF”表示无效
                buf[index++] = stream->gps_status;                         // 24  定位状态  BYTE    数据长度：1btyes
                index += bigw_32bit(&buf[index], stream->longitude);       // 25  经度  DWORD   数据长度：4btyes 精度：0.000001° per bit 偏移量：0 数据范围：0~180.000000° “0xFF,0xFF,0xFF,0xFF”表示无效
                index += bigw_32bit(&buf[index], stream->latitude);        // 29  纬度  DWORD   数据长度：4btyes 精度：0.000001 度  per bit 偏移量：0 数据范围：0~180.000000° “0xFF,0xFF,0xFF,0xFF”表示无效
                index += bigw_32bit(&buf[index], stream->mileages_total);  // 33 累计里程 （总行驶里程） DWORD  km 数据长度：4btyes 精度：0.1km per bit 偏移量：0 “0xFF,0xFF,0xFF,0xFF”表示无效

                break;
            case MSG_STREAM_ATT: // 0x80  补充数据流
                if((index+sizeof (struct shanghai_data_att))>_size) return ERR_ENCODE_PACKL;
                pr_debug("MSG_STREAM_ATT : %d \n", index); //fflush(stdout);
                att = (const struct shanghai_data_att *)container_of(nmsg, struct shanghai_data_att, head);
                buf[index++] = nmsg->type_msg;
                buf[index++] = att->Nm_mode;                            // 0  发动机扭矩模式  BYTE    0：超速失效 1：转速控制 2：扭矩控制 3：转速/扭矩控制 9：正常
                buf[index++] = att->accelerator;                        // 1  油门踏板  BYTE  % 数据长度：1btyes 精度：0.4%/bit 偏移量：0 数据范围：0~100% “0xFF”表示无效
                index += bigw_32bit(&buf[index], att->oil_consume);     // 2 累计油耗 （总油耗） DWORD  L 数据长度：4btyes 精度：0.5L per bit 偏移量：0 数据范围：0~2 105 540 607.5L “0xFF,0xFF,0xFF,0xFF”表示无效
                buf[index++] = att->urea_tank_temp;                     // 6  尿素箱温度  BYTE  ℃ 数据长度：1btyes 精度：1 ℃/bit 偏移量：-40 数据范围：-40~210℃ “0xFF”表示无效
                index += bigw_32bit(&buf[index], att->mlh_urea_actual); // 7  实际尿素喷射量  DWORD  ml/h 数据长度：4btyes 精度：0.01 ml/h per bit 偏移量：0 数据范围：0 “0xFF,0xFF,0xFF,0xFF”表示无效
                index += bigw_32bit(&buf[index], att->mlh_urea_total);  // 11 累计尿素消耗 （总尿素消耗） DWORD  g 数据长度：4btyes 精度：1 g per bit 偏移量：0 数据范围：0  “0xFF,0xFF,0xFF,0xFF”表示无效
                index += bigw_16bit(&buf[index], att->exit_gas_temp);   // 15  DPF 排气温度  WORD  ℃ 数据长度：2btyes 精度：0.03125 ℃ per bit 偏移量：-273 数据范围：-273~1734.96875℃ “0xFF,0xFF”表示无效
                // ;
                break;
            // 0x03-0x7F  预留
            // 0x81~0xFE  用户自定义
            default:
                break;
        }
        nmsg = nmsg->next;   // 下一个数据
    }
    return index; // len
}
// 补发信息上报
static int encode_msg_report_later(const struct shanghai_report_real *const msg, uint8_t buf[], const uint16_t _size)
{
    return encode_msg_report_real(msg, buf, _size);
}
// 自定义数据
static int encode_msg_userdef(const struct shanghai_userdef *const msg, uint8_t buf[], const uint16_t _size)
{
    if(msg->_dsize>_size) return ERR_ENCODE_PACKL;
    memcpy(buf, msg->data, msg->_dsize);
    return msg->_dsize;
}
/**
 * 通用编码函数,根据参数 msg_id 号调用相应子函数编码消息中的数据
 */
static int obd_encode_pack_general(const enum general_pack_type _pack_type, struct obd_agree_obj* const _obd_fops, const void * const msg, uint8_t buf[], const uint16_t _size)
{
    uint8_t bcc=0;
    uint16_t index=0;
    int data_len=0;
    uint8_t rsa_buffer[2048];
    struct general_pack_shanghai * const _pack = &_obd_fops->_gen_pack;
    _pack->cmd = get_cmd(_pack_type);
    if(_size<sizeof (struct general_pack_shanghai)) return ERR_ENCODE_PACKL;
    memset(buf, 0, _size);
    index=0;
    pr_debug("encode_pack_general...\n");
    buf[index++] = _pack->start[0];         // 0  起始符  STRING  固定为 ASCII 字符’##’，用“0x23，0x23”表示
    buf[index++] = _pack->start[1];         // 0  起始符  STRING  固定为 ASCII 字符’##’，用“0x23，0x23”表示
    buf[index++]   = _pack->cmd;              // 2  命令单元  BYTE  命令单元定义见  表 A2  命令单元定义
    memcpy(&buf[index], _pack->VIN, 17);    // 3  车辆识别号  STRING 车辆识别码是识别的唯一标识，由 17 位字码组成，字码应符合 GB16735 中 4.5 的规定
    BUILD_BUG_ON(sizeof (_pack->VIN)-1 != 17);
    index += 17;
    buf[index++] = _pack->soft_version;     // 20  终端软件版本号  BYTE  终端软件版本号有效值范围 0~255
    buf[index++] = _pack->ssl;              // 21  数据加密方式  BYTE
    index += bigw_16bit(&buf[index], _pack->data_len); // 22  数据单元长度  WORD
    // 24  数据单元
    switch(_pack->cmd)
    {
        case CMD_LOGIN:        // 车辆登入
            data_len = encode_msg_login((const struct shanghai_login *const)msg, &buf[index], _size-index-1);
            break;
        case CMD_REPORT_REAL:  // 实时信息上报
            data_len = encode_msg_report_real((const struct shanghai_report_real *const)msg, &buf[index], _size-index-1);
            break;
        case CMD_REPORT_LATER: // 补发信息上报
            data_len = encode_msg_report_later((const struct shanghai_report_real *const)msg, &buf[index], _size-index-1);
            break;
        case CMD_LOGOUT:       // 车辆登出
            data_len = encode_msg_logout((const struct shanghai_logout *const)msg, &buf[index], _size-index-1);
            break;
        case CMD_UTC:          // 终端校时
            data_len = 0;      // 车载终端校时的数据单元为空。
            break;
        case CMD_USERDEF:      // 用户自定义
            data_len = encode_msg_userdef((const struct shanghai_userdef *const)msg, &buf[index], _size-index-1);
            break;
        default:
            data_len = 0;
            break;
    }
    if(data_len<0) return ERR_ENCODE_PACKL;
    // 数据加密
    switch (_pack->ssl)
    {
    /*
        {0x01, "INFO"},  // 0x01：数据不加密
        {0x02, "RSA" },  // 0x02：数据经过 RSA 算法加密
        {0x03, "SM2" },  // 0x03：数据经过国密 SM2 算法加密
        {0xFF, "NULL"},  // “0xFE”标识异常，“0xFF”表示无效，其他预留
     */
        case 0x02:  // RSA
            pr_debug("encode_pack_general RSA data_len: %d\n", data_len);
            data_len = rsa_encrypt(rsa_buffer, sizeof(rsa_buffer), &buf[index], data_len);
            pr_debug("after encode_pack_general RSA data_len: %d\n", data_len);
            if(data_len>0) memcpy(&buf[index], rsa_buffer, data_len);
            break;
        case 0x03:   // SM2
            ;
            break;
        case 0x01:
        default:
            break;
    }
    if(data_len<0) return ERR_ENCODE_PACKL;
    _pack->data_len = (uint16_t)data_len;
    bigw_16bit(&buf[index-2], _pack->data_len); // 22  数据单元长度  WORD
    pr_debug("encode_pack_general data_len: %d\n", data_len);
    index += data_len;
    // 倒数第 1  校验码  BYTE 采用 BCC（异或校验）法，校验范围聪明星单元的第一个字节开始，同后一个字节异或，直到校验码前一字节为止，校验码占用一个字节
    //bcc = BCC_check_code(&buf[1], index-1);
    bcc = BCC_check_code(&buf[2], index-2);  // start: 2 byte
    pr_debug("bcc[%d]:%02X \n", index-1, bcc);
    buf[index++] = bcc;
    return index; // len
}
// 解码
static int decode_msg_login(struct shanghai_login *msg, const uint8_t data[], const uint16_t _size)
{
    //pthread_mutex_init();
    uint16_t index=0;
    uint16_t data_len = 0;
    index=0;
    pr_debug("decode_msg_login...\n");
    // 数据的总字节长度,  UTC + count + ICCID
    data_len = sizeof (msg->UTC)-1 + sizeof (msg->count) + sizeof (msg->ICCID) -1;
    if(data_len>_size) return ERR_DECODE_LOGIN;    // error
    // 0  数据采集时间  BYTE[6]  时间定义见 A4.4
    memcpy(msg->UTC, &data[index], 6);
    BUILD_BUG_ON(sizeof (msg->UTC)-1 != 6);
    index += 6;
    // 6  登入流水号  WORD 车载终端每登入一次，登入流水号自动加 1，从 1开始循环累加，最大值为 65531，循环周期为天。
    msg->count = merge_16bit(data[index], data[index+1]);
    index += 2;
    // 10  SIM 卡号  STRING SIM 卡 ICCID 号（ICCID 应为终端从 SIM 卡获取的值，不应人为填写或修改）
    memcpy(msg->ICCID, &data[index], 20);// SIM 卡号  2  String[20]  20  SIM 卡的 ICCID 号（集成电路卡识别码）。由 20 位数字的 ASCII 码构成。
    BUILD_BUG_ON(sizeof (msg->ICCID)-1 != 20);
    index += 20;
    pr_debug("UTC:%s \n", msg->UTC);
    pr_debug("count:%d \n", msg->count);
    pr_debug("ICCID:%s \n", msg->ICCID);
    return index;
}
/*static int decode_msg_login_yj(struct yunjing_login *msg, const uint8_t data[], const uint16_t _size)
{
    //pthread_mutex_init();
    uint16_t index=0;
    uint16_t data_len = 0;
    index=0;
    pr_debug("decode_msg_login...\n");
    // 数据的总字节长度,  UTC + count + ICCID
    data_len = sizeof (msg->UTC)-1 + sizeof (msg->count) + sizeof (msg->sn) -1;
    if(data_len>_size) return ERR_DECODE_LOGIN;    // error
    // 0  数据采集时间  BYTE[6]  时间定义见 A4.4
    memcpy(msg->UTC, &data[index], 6);
    BUILD_BUG_ON(sizeof (msg->UTC)-1 != 6);
    index += 6;
    // 6  登入流水号  WORD 车载终端每登入一次，登入流水号自动加 1，从 1开始循环累加，最大值为 65531，循环周期为天。
    msg->count = merge_16bit(data[index], data[index+1]);
    index += 2;
    // 8  设备序列号由 6 位固定编码和 12 位厂家自定义序号(不重复)共 18 位组成。详情看附件设备序列号要求
    memcpy(msg->sn, &data[index], 18);// 8  设备序列号由 6 位固定编码和 12 位厂家自定义序号(不重复)共 18 位组成。详情看附件设备序列号要求
    BUILD_BUG_ON(sizeof (msg->sn)-1 != 18);
    index += 18;
    pr_debug("UTC:%s \n", msg->UTC);
    pr_debug("count:%d \n", msg->count);
    pr_debug("SN:%s \n", msg->sn);
    return index;
}*/
static int decode_msg_logout(struct shanghai_logout *msg, const uint8_t data[], const uint16_t _size)
{
    uint16_t index=0;
    uint16_t data_len = 0;
    index=0;
    pr_debug("decode_msg_logout...\n");
    // 数据的总字节长度
    data_len = sizeof (msg->UTC)-1 + sizeof (msg->count);
    if(data_len>_size) return ERR_DECODE_LOGOUT;    // error
    // 0  数据采集时间  BYTE[6]  时间定义见 A4.4
    memcpy(msg->UTC, &data[index], 6);
    BUILD_BUG_ON(sizeof (msg->UTC)-1 != 6);
    index += 6;
    // 6  登入流水号  WORD 车载终端每登入一次，登入流水号自动加 1，从 1开始循环累加，最大值为 65531，循环周期为天。
    msg->count = merge_16bit(data[index], data[index+1]);
    index += 2;
    pr_debug("UTC:%s \n", msg->UTC);
    pr_debug("count:%d \n", msg->count);
    return index;
}
/*static int decode_msg_logout_yj(struct yunjing_logout *msg, const uint8_t data[], const uint16_t _size)
{
    uint16_t index=0;
    uint16_t data_len = 0;
    index=0;
    pr_debug("decode_msg_logout...\n");
    // 数据的总字节长度
    data_len = sizeof (msg->UTC)-1 + sizeof (msg->count) + sizeof (msg->sn)-1;
    if(data_len>_size) return ERR_DECODE_LOGOUT;    // error
    // 0  数据采集时间  BYTE[6]  时间定义见 A4.4
    memcpy(msg->UTC, &data[index], 6);
    BUILD_BUG_ON(sizeof (msg->UTC)-1 != 6);
    index += 6;
    // 6  登入流水号  WORD 车载终端每登入一次，登入流水号自动加 1，从 1开始循环累加，最大值为 65531，循环周期为天。
    msg->count = merge_16bit(data[index], data[index+1]);
    index += 2;
    // 8  设备序列号由 6 位固定编码和 12 位厂家自定义序号(不重复)共 18 位组成。详情看附件设备序列号要求
    memcpy(msg->sn, &data[index], 18);// 8  设备序列号由 6 位固定编码和 12 位厂家自定义序号(不重复)共 18 位组成。详情看附件设备序列号要求
    BUILD_BUG_ON(sizeof (msg->sn)-1 != 18);
    index += 18;
    pr_debug("UTC:%s \n", msg->UTC);
    pr_debug("count:%d \n", msg->count);
    return index;
}*/
//struct shanghai_data_obd *save_obd=NULL;
static int decode_msg_report_real(void* msg_buf, const uint16_t _msize, const uint8_t data[], const uint16_t _size)
{
    uint16_t index=0;
    uint16_t data_len=0;
    uint8_t i=0;
    struct shanghai_report_real *const msg = (struct shanghai_report_real *const )msg_buf;
    // 缓冲器指针，msg_buf必须足够大，链表将直接使用 msg_buf上面的空间，以避免动态申请内存.
    uint8_t* ptr = (uint8_t*)msg_buf;
    struct report_head* nmsg=NULL;  // next msg
    struct report_head* fault=NULL;
    struct shanghai_data_obd *obd=NULL;
    struct shanghai_data_stream *stream=NULL;
    struct shanghai_data_att *att=NULL;
    uint8_t fault_count=0;
    // one, 数据头
    data_len = sizeof (msg->UTC)-1 + sizeof (msg->count);
    //pr_debug("decode_msg_report_real data_len:%d _size:%d \n", data_len, _size); fflush(stdout);
    if(data_len>_size) return ERR_DECODE_PACKL;
    //pr_debug("decode_msg_report_real _size:%d _msize:%d \n", _size, _msize); fflush(stdout);
    if(_size>_msize) return ERR_DECODE_PACKL;
    //memset(buf, 0, _size);
    index=0;
    memcpy(msg->UTC, &data[index], sizeof (msg->UTC)-1);     // 数据采集时间  6  BYTE[6]  时间定义见 A4.4
    BUILD_BUG_ON(sizeof (msg->UTC)-1 != 6);
    index += 6;
    msg->count = merge_16bit(data[index], data[index+1]); index += 2;           // 信息流水号  2  WORD
    // 消息，可包含多条
    //if(NULL==msg->msg) return index; // 无消息
    //nmsg = msg->msg;
    ptr += sizeof (struct shanghai_report_real);   // 移动缓冲区指针
    //pr_debug("decode_msg_report_real index:%d _size:%d\n", index, _size); fflush(stdout);
    while(index<_size)
    {
        //pr_debug("decode_msg_report_real2 index:%d _size:%d switch:%d\n", index, _size, data[index]); fflush(stdout);
        switch (data[index])   // 消息类型
        {
            // 1） OBD 信息数据格式和定义见表 A.6 所示。
            case MSG_OBD:       // 0x01  OBD 信息
                pr_debug("\nMSG_OBD : %d \n", index); //fflush(stdout);
                obd = (struct shanghai_data_obd *) ptr;
                //pr_debug("obd addr: 0X%08X \n", obd);
                //save_obd = obd;
                //pr_debug("MSG_OBD ...1 \n"); fflush(stdout);
                ptr += sizeof (struct shanghai_data_obd);   // 移动缓冲区指针
                //pr_debug("MSG_OBD ...2 \n"); fflush(stdout);
                obd->head.type_msg = data[index++];
                obd->protocol = data[index++];                                     // OBD 诊断协议  1  BYTE 有效范围 0~2，“0”代表 IOS15765，“1”代表IOS27145，“2”代表 SAEJ1939，“0xFE”表示无效。
                //pr_debug("MSG_OBD ...3 \n"); fflush(stdout);
                obd->MIL = data[index++];                                          // MIL 状态  1  BYTE 有效范围 0~1，“0”代表未点亮，“1”代表点亮。“0xFE”表示无效。
                obd->status = merge_16bit(data[index], data[index+1]); index += 2; // 诊断支持状态  2  WORD
                obd->ready = merge_16bit(data[index], data[index+1]); index += 2;  // 诊断就绪状态  2  WORD
                // 车辆识别码（VIN） 17  STRING
                memcpy(obd->VIN, &data[index], sizeof (obd->VIN)-1);
                BUILD_BUG_ON(sizeof (obd->VIN)-1 != 17);
                index += 17;
                // 软件标定识别号 18  STRING 软件标定识别号由生产企业自定义，字母或数字组成，不足后面补字符“0”。
                memcpy(obd->SVIN, &data[index], sizeof (obd->SVIN)-1);
                BUILD_BUG_ON(sizeof (obd->SVIN)-1 != 18);
                index += 18;
                // 标定验证码（CVN） 18  STRING 标定验证码由生产企业自定义，字母或数字组成，不足后面补字符“0”。
                memcpy(obd->CVN, &data[index], sizeof (obd->CVN)-1);
                BUILD_BUG_ON(sizeof (obd->CVN)-1 != 18);
                index += 18;
                // IUPR 值  36  DSTRING  定义参考 SAE J 1979-DA 表 G11.
                //memcpy(obd->IUPR, &data[index], sizeof (obd->IUPR)-1);
                for(i=0; i<18; i++)
                {
                    obd->IUPR[i] = merge_16bit(data[index], data[index+1]); index += 2;
                }
                BUILD_BUG_ON(sizeof (obd->IUPR) != 36);
                //index += 36;
//                pr_debug("MSG_OBD VIN: %s \n", obd->VIN);
//                pr_debug("MSG_OBD SVIN: %s \n", obd->SVIN);
//                pr_debug("MSG_OBD CVN: %s \n", obd->CVN);
                obd->fault_total = data[index++];                                   // 故障码总数  1  BYTE  有效值范围：0~253，“0xFE”表示无效。
                // 故障码信息列表 ∑每个故障码信息长度 N*BYTE（4） 每个故障码为四字节，可按故障实际顺序进行排序。
                fault = &(obd->fault_list);
                fault->next = NULL;
                fault_count=0;
                //pr_debug("MSG_OBD ... index:%d fault_total:%d \n", index, obd->fault_total); fflush(stdout);
                //while(fault_count<obd->fault_total)
                for(fault_count=0; fault_count<obd->fault_total; fault_count++)
                {
                    //printf("decode_msg_report_real[%d]: %d \n", obd->fault_total, fault_count); fflush(stdout);
                    fault->data = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]);  // 故障码, BYTE（4）
                    index += 4;
#if 0
                    fault_count++;
                    if(fault_count<obd->fault_total)
                    {
                        fault->next = (struct report_head* )ptr;
                        ptr += sizeof (struct report_head);   // 移动缓冲区指针
                    }
                    else
                    {
                        break;
                    }
#else
                    fault->next = (struct report_head* )ptr;
                    ptr += sizeof (struct report_head);   // 移动缓冲区指针
#endif
                    fault = fault->next;  // 下一个数据
                    fault->next = NULL;
                }
                //pr_debug("MSG_OBD ...break index:%d \n", index); fflush(stdout);
                if(NULL==msg->msg)
                {
                    nmsg=&obd->head;
                    msg->msg = nmsg;
                }
                else
                {
                    nmsg->next=&obd->head;
                    nmsg = nmsg->next;   // 下一个数据
                }
                nmsg->next = NULL;
                break;
            // 2）数据流信息数据格式和定义见表 A.7 所示，补充数据流信息数据格式和定义见表 A.8 所示。
            case MSG_STREAM:     // 0x02  数据流信息
                pr_debug("\nMSG_STREAM : %d \n", index); //fflush(stdout);
                stream = (struct shanghai_data_stream *) ptr;
                //pr_debug("stream addr: 0X%08X \n", stream);
                ptr += sizeof (struct shanghai_data_stream);                             // 移动缓冲区指针
                stream->head.type_msg = data[index++];
                stream->speed = merge_16bit(data[index], data[index+1]); index += 2;     // 0  车速  WORD  km/h 数据长度：2btyes 精度：1/256km/h/ bit 偏移量：0 数据范围：0~250.996km/h “0xFF,0xFF”表示无效
                stream->kPa = data[index++];                                             // 2  大气压力  BYTE  kPa 数据长度：1btyes 精度：0.5/bit 偏移量：0 数据范围：0~125kPa “0xFF”表示无效
                stream->Nm = data[index++];                                              // 3  发动机净输出扭矩（实际扭矩百分比）  BYTE  %  数据长度：1btyes精度：1%/bit 偏移量：-125 数据范围：-125~125% “0xFF”表示无效
                stream->Nmf = data[index++];                                             // 4 摩擦扭矩（摩擦扭矩百分比） BYTE  % 数据长度：1btyes 精度：1%/bit 偏移量：-125 数据范围：-1250~125% “0xFF”表示无效
                stream->rpm = merge_16bit(data[index], data[index+1]); index += 2;       // 5  发动机转速  WORD  rpm 数据长度：2btyes 精度：0.125rpm/bit 偏移量：0 数据范围：0~8031.875rpm “0xFF,0xFF”表示无效
                stream->Lh = merge_16bit(data[index], data[index+1]); index += 2;        // 7  发动机燃料流量  WORD  L/h 数据长度：2btyes 精度：0.05L/h 偏移量：0 数据范围：0~3212.75L/h “0xFF,0xFF”表示无效
                stream->ppm_up = merge_16bit(data[index], data[index+1]); index += 2;    // 9 SCR 上游 NOx 传感器输出值（后处理上游氮氧浓度） WORD  ppm 数据长度：2btyes 精度：0.05ppm/bit 偏移量：-200 数据范围：-200~3212.75ppm “0xFF,0xFF”表示无效
                stream->ppm_down = merge_16bit(data[index], data[index+1]); index += 2;  // 11 SCR 下游 NOx 传感器输出值（后处理下游氮氧浓度） WORD  ppm 数据长度：2btyes 精度：0.05ppm/bit 偏移量：-200 数据范围：-200~3212.75ppm  “0xFF,0xFF”表示无效
                stream->urea_level = data[index++];                                      // 13 反应剂余量 （尿素箱液位） BYTE  % 数据长度：1btyes 精度：0.4%/bit 偏移量：0 数据范围：0~100% “0xFF”表示无效
                stream->kgh = merge_16bit(data[index], data[index+1]); index += 2;       // 14  进气量  WORD  kg/h 数据长度：2btyes 精度：0.05kg/h per bit 偏移量：0 数据范围：0~3212.75ppm “0xFF,0xFF”表示无效
                stream->SCR_in = merge_16bit(data[index], data[index+1]); index += 2;    // 16 SCR 入口温度（后处理上游排气温度） WORD  ℃ 数据长度：2btyes 精度：0.03125 ℃ per bit 偏移量：-273 数据范围：-273~1734.96875℃ “0xFF,0xFF”表示无效
                stream->SCR_out = merge_16bit(data[index], data[index+1]); index += 2;   // 18 SCR 出口温度（后处理下游排气温度） WORD  ℃ 数据长度：2btyes 精度：0.03125 ℃ per bit 偏移量：-273 数据范围：-273~1734.96875℃ “0xFF,0xFF”表示无效
                stream->DPF = merge_16bit(data[index], data[index+1]); index += 2;       // 20 DPF 压差（或 DPF排气背压） WORD  kPa 数据长度：2btyes 精度：0.1 kPa per bit 偏移量：0 数据范围：0~6425.5 kPa “0xFF,0xFF”表示无效
                stream->coolant_temp = data[index++];                                    // 22  发动机冷却液温度  BYTE  ℃ 数据长度：1btyes 精度：1 ℃/bit 偏移量：-40 数据范围：-40~210℃ “0xFF”表示无效
                stream->tank_level = data[index++];                                      // 23  油箱液位  BYTE  % 数据长度：1btyes 精度：0.4% /bit 偏移量：0 数据范围：0~100% “0xFF”表示无效
                stream->gps_status = data[index++];                                      // 24  定位状态  BYTE    数据长度：1btyes
                stream->longitude = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]);       // 25  经度  DWORD   数据长度：4btyes 精度：0.000001° per bit 偏移量：0 数据范围：0~180.000000° “0xFF,0xFF,0xFF,0xFF”表示无效
                index += 4;
                stream->latitude = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]);        // 29  纬度  DWORD   数据长度：4btyes 精度：0.000001 度  per bit 偏移量：0 数据范围：0~180.000000° “0xFF,0xFF,0xFF,0xFF”表示无效
                index += 4;
                stream->mileages_total = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]);  // 33 累计里程 （总行驶里程） DWORD  km 数据长度：4btyes 精度：0.1km per bit 偏移量：0 “0xFF,0xFF,0xFF,0xFF”表示无效
                index += 4;
                if(NULL==msg->msg)
                {
                    nmsg=&stream->head;
                    msg->msg = nmsg;
                }
                else
                {
                    nmsg->next=&stream->head;
                    nmsg = nmsg->next;   // 下一个数据
                }
                nmsg->next = NULL;
                //pr_debug("MSG_STREAM ...break index:%d \n", index); fflush(stdout);
                break;
            case MSG_STREAM_ATT: // 0x80  补充数据流
                pr_debug("\nMSG_STREAM_ATT : %d \n", index); //fflush(stdout);
                att = (struct shanghai_data_att *)ptr;
                //pr_debug("att addr: 0X%08X \n", att);
                ptr += sizeof (struct shanghai_data_att);                     // 移动缓冲区指针
                att->head.type_msg = data[index++];
                att->Nm_mode = data[index++];                                 // 0  发动机扭矩模式  BYTE    0：超速失效 1：转速控制 2：扭矩控制 3：转速/扭矩控制 9：正常
                att->accelerator = data[index++];                             // 1  油门踏板  BYTE  % 数据长度：1btyes 精度：0.4%/bit 偏移量：0 数据范围：0~100% “0xFF”表示无效
                att->oil_consume = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]);     // 2 累计油耗 （总油耗） DWORD  L 数据长度：4btyes 精度：0.5L per bit 偏移量：0 数据范围：0~2 105 540 607.5L “0xFF,0xFF,0xFF,0xFF”表示无效
                index += 4;
                att->urea_tank_temp = data[index++];                          // 6  尿素箱温度  BYTE  ℃ 数据长度：1btyes 精度：1 ℃/bit 偏移量：-40 数据范围：-40~210℃ “0xFF”表示无效
                att->mlh_urea_actual = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); // 7  实际尿素喷射量  DWORD  ml/h 数据长度：4btyes 精度：0.01 ml/h per bit 偏移量：0 数据范围：0 “0xFF,0xFF,0xFF,0xFF”表示无效
                index += 4;
                att->mlh_urea_total = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); // 11 累计尿素消耗 （总尿素消耗） DWORD  g 数据长度：4btyes 精度：1 g per bit 偏移量：0 数据范围：0  “0xFF,0xFF,0xFF,0xFF”表示无效
                index += 4;
                att->exit_gas_temp = merge_16bit(data[index], data[index+1]); // 15  DPF 排气温度  WORD  ℃ 数据长度：2btyes 精度：0.03125 ℃ per bit 偏移量：-273 数据范围：-273~1734.96875℃ “0xFF,0xFF”表示无效
                index += 2;
                if(NULL==msg->msg)
                {
                    nmsg=&att->head;
                    msg->msg = nmsg;
                }
                else
                {
                    nmsg->next=&att->head;
                    nmsg = nmsg->next;   // 下一个数据
                }
                nmsg->next = NULL;
                break;
            // 0x03-0x7F  预留
            // 0x81~0xFE  用户自定义
            default:
                pr_debug("\nERR_DECODE_OBD[%02X] : %d \n", data[index]&0xFF, index); //fflush(stdout);
                return ERR_DECODE_OBD;
                //break;
        }
//        pr_debug("save addr: 0X%08X \n", save_obd);
//        pr_debug("save_MSG_OBD VIN: %s \n", save_obd->VIN);
//        pr_debug("save_MSG_OBD SVIN: %s \n", save_obd->SVIN);
//        pr_debug("save_MSG_OBD CVN: %s \n", save_obd->CVN);
        //nmsg = nmsg->next;   // 下一个数据
    }
    //pr_debug("msg.msg : %d \n", (NULL!=msg->msg)); fflush(stdout);
    return index; // len
}

static int decode_msg_userdef(struct shanghai_userdef *msg, const uint8_t data[], const uint16_t _size)
{
    msg->data = data;
    msg->_dsize = _size;
    return _size;
}
/**
 * 通用解包函数,根据接收数据中的 msg_id 号调用相应子函数解析消息中的数据
 */
static int obd_decode_pack_general(struct obd_agree_obj* const _obd_fops, const uint8_t data[], const uint16_t _dsize, void * const msg_buf, const uint16_t _msize)
{
    uint8_t bcc=0;
    uint16_t index=0;
    int msg_len = 0;
    int data_len=0;
    uint8_t rsa_buffer[2048];
    const uint8_t* pdata=NULL;
    struct general_pack_shanghai * const _pack = &_obd_fops->_gen_pack;
    index=0;
    // 0  起始符  STRING  固定为 ASCII 字符’##’，用“0x23，0x23”表示
    //memset(_pack, 0, sizeof (struct general_pack_shanghai));
    memset(msg_buf, 0, _msize);
    _pack->start[0] = data[index++];
    _pack->start[1] = data[index++];
    if(('#'!=_pack->start[0]) || ('#'!=_pack->start[1])) return ERR_DECODE_PACKS; // 包头错误
    _pack->cmd = data[index++];                                // 2  命令单元  BYTE  命令单元定义见  表 A2  命令单元定义
    memcpy(_pack->VIN, &data[index], 17);                      // 3  车辆识别号  STRING 车辆识别码是识别的唯一标识，由 17 位字码组成，字码应符合 GB16735 中 4.5 的规定
    index += 17;
    _pack->soft_version = data[index++];                       // 20  终端软件版本号  BYTE  终端软件版本号有效值范围 0~255
    _pack->ssl = data[index++];                                // 21  数据加密方式  BYTE
    _pack->data_len = merge_16bit(data[index], data[index+1]); // 22  数据单元长度  WORD 数据单元长度是数据单元的总字节数，有效范围：0~65531
    index += 2;
    //printf("decode_pack_general _dsize:%d | %d \n\n", _dsize, (_pack->data_len+index+1)); fflush(stdout);
    if(_dsize<(_pack->data_len+index+1)) return  ERR_DECODE_PACKL; // 包长度不够
    //printf("decode_pack_general _msize:%d | %d \n", _msize, (_pack->data_len)); fflush(stdout);
    if(_msize<(_pack->data_len)) return  ERR_DECODE_PACKL;         // 数据缓存长度不够
    //pr_debug("decode_pack_general switch %d \n", index); fflush(stdout);
    // 倒数第 1  校验码  BYTE 采用 BCC（异或校验）法，校验范围聪明星单元的第一个字节开始，同后一个字节异或，直到校验码前一字节为止，校验码占用一个字节
    //bcc = BCC_check_code(&data[2], index+_pack->data_len-1);
    bcc = BCC_check_code(&data[2], index+_pack->data_len-2);  // start: 2 byte
    _pack->BCC = data[index+_pack->data_len];
    //printf("BCC[%d]:%02X bcc:%02X \n\n", index+_pack->data_len, _pack->BCC, bcc); fflush(stdout);
    if(bcc != _pack->BCC) return ERR_DECODE_PACKBCC;                // BCC 校验错误
#if 1
    // 数据解谜
    data_len = _pack->data_len;
    switch (_pack->ssl)
    {
    /*
        {0x01, "INFO"},  // 0x01：数据不加密
        {0x02, "RSA" },  // 0x02：数据经过 RSA 算法加密
        {0x03, "SM2" },  // 0x03：数据经过国密 SM2 算法加密
        {0xFF, "NULL"},  // “0xFE”标识异常，“0xFF”表示无效，其他预留
     */
        case 0x02:  // RSA
            pr_debug("encode_pack_general RSA data_len: %d\n", data_len);
            data_len = rsa_decrypt(rsa_buffer, sizeof(rsa_buffer), &data[index], data_len);
            pdata = rsa_buffer;
            break;
        case 0x03:   // SM2
            pdata = &data[index];
            break;
        case 0x01:
        default:
            pdata = &data[index];
            break;
    }
    // 解码数据
    msg_len = 0;
    switch(_pack->cmd)
    {
        case CMD_LOGIN:          // 车辆登入  上行
            pr_debug("decode_pack_general CMD_LOGIN \n"); //fflush(stdout);
            msg_len = decode_msg_login((struct shanghai_login *)msg_buf, pdata, data_len);
            break;
        case CMD_REPORT_REAL:    // 实时信息上报  上行
            pr_debug("decode_pack_general CMD_REPORT_REAL \n"); //fflush(stdout);
            msg_len = decode_msg_report_real(msg_buf, _msize, pdata, data_len);
//            pr_debug("save_MSG_OBD VIN: %s \n", save_obd->VIN);
//            pr_debug("save_MSG_OBD SVIN: %s \n", save_obd->SVIN);
//            pr_debug("save_MSG_OBD CVN: %s \n", save_obd->CVN);
            break;
        case CMD_REPORT_LATER:   // 补发信息上报  上行
            pr_debug("decode_pack_general CMD_REPORT_LATER \n"); //fflush(stdout);
            msg_len = decode_msg_report_real(msg_buf, _msize, pdata, data_len);
            break;
        case CMD_LOGOUT:         // 车辆登出  上行
            pr_debug("decode_pack_general CMD_LOGOUT \n"); //fflush(stdout);
            msg_len = decode_msg_logout((struct shanghai_logout *)msg_buf, pdata, data_len);
            //printf("decode_pack_general CMD_LOGOUT [%d %d]\n", msg_len, data_len);
            break;
        case CMD_UTC:            // 终端校时  上行
            pr_debug("decode_pack_general CMD_UTC \n"); //fflush(stdout);
            msg_len = 0;
            break;
        case CMD_USERDEF:      // 用户自定义
            pr_debug("decode_pack_general CMD_USERDEF \n"); //fflush(stdout);
            msg_len = decode_msg_userdef((struct shanghai_userdef *)msg_buf, pdata, data_len);
            break;
        default:
            printf("decode_pack_general default \n"); //fflush(stdout);
            break;
    }
#else
    // 解码数据
    msg_len = 0;
    switch(_pack->cmd)
    {
        case CMD_LOGIN:          // 车辆登入  上行
            pr_debug("decode_pack_general CMD_LOGIN \n"); fflush(stdout);
            msg_len = decode_msg_login((struct shanghai_login *)msg_buf, &data[index], _pack->data_len);
            break;
        case CMD_REPORT_REAL:    // 实时信息上报  上行
            pr_debug("decode_pack_general CMD_REPORT_REAL \n"); fflush(stdout);
            msg_len = decode_msg_report_real(msg_buf, _msize, &data[index], _pack->data_len);
//            pr_debug("save_MSG_OBD VIN: %s \n", save_obd->VIN);
//            pr_debug("save_MSG_OBD SVIN: %s \n", save_obd->SVIN);
//            pr_debug("save_MSG_OBD CVN: %s \n", save_obd->CVN);
            break;
        case CMD_REPORT_LATER:   // 补发信息上报  上行
            pr_debug("decode_pack_general CMD_REPORT_LATER \n"); fflush(stdout);
            msg_len = decode_msg_report_real(msg_buf, _msize, &data[index], _pack->data_len);
            break;
        case CMD_LOGOUT:         // 车辆登出  上行
            pr_debug("decode_pack_general CMD_LOGOUT \n"); fflush(stdout);
            msg_len = decode_msg_logout((struct shanghai_logout *)msg_buf, &data[index], _pack->data_len);
            break;
        case CMD_UTC:            // 终端校时  上行
            pr_debug("decode_pack_general CMD_UTC \n"); fflush(stdout);
            msg_len = 0;
            break;
        case CMD_USERDEF:      // 用户自定义
            pr_debug("decode_pack_general CMD_USERDEF \n"); fflush(stdout);
            msg_len = decode_msg_userdef((struct shanghai_userdef *)msg_buf, &data[index], _pack->data_len);
            break;
        default:
            pr_debug("decode_pack_general default \n"); fflush(stdout);
            break;
    }
#endif
    //pr_debug("decode_pack_general msg_len %d \n\n", msg_len); fflush(stdout);
    if(msg_len<0) return msg_len;
    if(msg_len!=data_len) return ERR_DECODE_OBD;
    index += _pack->data_len;
    // 倒数第 1  校验码  BYTE 采用 BCC（异或校验）法，校验范围聪明星单元的第一个字节开始，同后一个字节异或，直到校验码前一字节为止，校验码占用一个字节
    //bcc = BCC_check_code(&data[1], index-1);
    bcc = BCC_check_code(&data[2], index-2);  // start: 2 byte
    _pack->BCC = data[index++];
    if(bcc != _pack->BCC) return ERR_DECODE_PACKBCC;                // BCC 校验错误
    //pr_debug("decode_pack_general return %d \n\n", index); fflush(stdout);
    return  index;
}

static const struct obd_agree_fops _obd_agree_fops_shanghai = {
    .constructed = obd_agree_fops_constructed,
    .init = init,
    .protocol = PRO_TYPE_SHH,
    .check_pack = check_pack_general,
    .decode = handle_decode,
    .userdef_encode = userdef_encode,
    .userdef_decode = userdef_decode,
    .upload_push = upload_push,
    .decode_server = obd_fops_decode_server,
    .decode_client = obd_fops_decode_client,
    .encode_pack_general = obd_encode_pack_general,
    .decode_pack_general = obd_decode_pack_general,
    .protocol_server = obj_obd_agree_shanghai_server,
    .protocol_client = obd_protocol_client_shanghai,
    .agree_des = "上海OBD协议",
    //.vin = &_obd_vin_fops,
    .base = &_obd_fops_base,
};
struct obd_agree_obj obd_agree_obj_shanghai = {
    .fops = &_obd_agree_fops_shanghai,
    //.protocol = PRO_TYPE_SHH,
    //._tbuf = "\0",
    //._tlen = 0,
    //._print_buf = "\0",
    ._print = 0,
    ._relay = 0,
    //.protocol_server = obj_obd_agree_shanghai_server,
    //.protocol_client = obd_protocol_client_shanghai,
};

