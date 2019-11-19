/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : obd_agree_yunjing.c
* Author             : Merafour
* Last Modified Date : 11/12/2019
* Description        : 《上海市车载排放诊断（OBD）系统在线接入技术指南（试行）发布稿.pdf》.
*                    :  云景客户修改其中部分指令
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#include "obd_agree_yunjing.h"
#include "encrypt.h"

#include <string.h>
//#include <memory.h>

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

#define  DECODE_SERVER   1    // server 端解码
//#undef   DECODE_SERVER

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
#if 0
#define container_of(ptr, type, member) ({          \
    const typeof(((type *)0)->member)*__mptr = (ptr);    \
             (type *)((char *)__mptr - offsetof(type, member)); })
#else
#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))
#endif

// cmd
static enum cmd_unit_yunjing get_cmd(const enum general_pack_type _pack_type)
{
    enum cmd_unit_yunjing _cmd = CMD_YJ_NULL;
    switch(_pack_type)
    {
        case GEN_PACK_LOGIN:        // 车辆登入
            _cmd = CMD_LOGIN_YJ;
            break;
        case GEN_PACK_REPORT_REAL:  // 实时信息上报
            _cmd = CMD_REPORT_REAL_YJ;
            break;
        case GEN_PACK_REPORT_LATER: // 补发信息上报
            _cmd = CMD_REPORT_LATER_YJ;
            break;
        case GEN_PACK_LOGOUT:       // 车辆登出
            _cmd = CMD_LOGOUT_YJ;
            break;
        case GEN_PACK_UTC:          // 终端校时
            _cmd = CMD_UTC_YJ;
            break;
        case GEN_PACK_USERDEF:      // 用户自定义
            _cmd = CMD_USERDEF_YJ;
            break;
        default:
            _cmd = CMD_YJ_NULL;
            break;
    }
    return _cmd;
}

static uint8_t SN[32];
#ifdef DECODE_SERVER
// 解码
static int decode_msg_login(struct yunjing_login *msg, const uint8_t data[], const uint16_t _size)
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
}
static int decode_msg_logout(struct yunjing_logout *msg, const uint8_t data[], const uint16_t _size)
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
}
//struct shanghai_data_obd *save_obd=NULL;
static int decode_msg_report_real(void* msg_buf, const uint16_t _msize, const uint8_t data[], const uint16_t _size)
{
    uint16_t index=0;
    uint16_t data_len=0;
    uint8_t i = 0;
    struct shanghai_report_real *const msg = (struct shanghai_report_real *)msg_buf;
    // 缓冲器指针，msg_buf必须足够大，链表将直接使用 msg_buf上面的空间，以避免动态申请内存.
    uint8_t* ptr = (uint8_t*)msg_buf;
    struct report_head* nmsg=NULL;  // next msg
    struct report_head* fault=NULL;
    //struct shanghai_data_obd *obd=NULL;
    //struct shanghai_data_stream *stream=NULL;
    //struct shanghai_data_att *att=NULL;
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
                pr_debug("\nMSG_OBD : %d \n", index); fflush(stdout);
                {
                    struct shanghai_data_obd* const obd = (struct shanghai_data_obd *) ptr;
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
                }
                break;
            // 2）数据流信息数据格式和定义见表 A.7 所示，补充数据流信息数据格式和定义见表 A.8 所示。
            case MSG_STREAM:     // 0x02  数据流信息
                pr_debug("\nMSG_STREAM : %d \n", index); fflush(stdout);
                {
                    struct shanghai_data_stream* const stream = (struct shanghai_data_stream *) ptr;
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
                }
                //pr_debug("MSG_STREAM ...break index:%d \n", index); fflush(stdout);
                break;
            case MSG_STREAM_ATT: // 0x80  补充数据流
                pr_debug("\nMSG_STREAM_ATT : %d \n", index); fflush(stdout);
                {
                    struct shanghai_data_att* const att = (struct shanghai_data_att *)ptr;
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
                }
                break;
            case MSG_SMOKE: // 0x81  包含烟雾的数据流信息(自定义)
                pr_debug("\nMSG_STREAM_ATT : %d \n", index); fflush(stdout);
                {
                    struct yunjing_smoke* const smoke = (struct yunjing_smoke *)ptr;
                    //pr_debug("att addr: 0X%08X \n", att);
                    ptr += sizeof (struct shanghai_data_att);                       // 移动缓冲区指针
                    smoke->head.type_msg = data[index++];
                    smoke->temperature = merge_16bit(data[index], data[index+1]);   // 0  烟雾排温, 数据长度：2 btyes, 精度：1℃ /bit,
                    index += 2;
                    smoke->fault = merge_16bit(data[index], data[index+1]);         // 2  OBD（烟雾故障码）,数据长度：2 btyes 精度：1/bit偏移量：0数据范围：“0xFF，0xFF”表示无效
                    index += 2;
                    smoke->kpa = merge_16bit(data[index], data[index+1]);           // 4  背压, 数据长度：2 btyes, 精度：1 kpa/bit,
                    index += 2;
                    smoke->m_l = merge_16bit(data[index], data[index+1]);           // 6  光吸收系数,数据长度：2 btyes,精度：0.01 m-l/bit
                    index += 2;
                    smoke->opacity = merge_16bit(data[index], data[index+1]);       // 8  不透光度,数据长度：2 btyes,精度：0.1%/bit
                    index += 2;
                    smoke->mg_per_m3 = merge_16bit(data[index], data[index+1]);     // 10 颗粒物浓度,数据长度：2 btyes,精度：0.1mg/m3 /bit
                    index += 2;
                    smoke->light_alarm = merge_16bit(data[index], data[index+1]);   // 12 光吸收系数超标报警,数据长度：2 btyes,精度：1
                    index += 2;
                    smoke->pressure_alarm = merge_16bit(data[index], data[index+1]); // 14 背压报警,数据长度：2 btyes,精度：1
                    index += 2;
                    smoke->ppm = merge_16bit(data[index], data[index+1]);            // 16 N0x 值,数据长度：2 btyes,精度：1ppm
                    index += 2;
                    if(NULL==msg->msg)
                    {
                        nmsg=&smoke->head;
                        msg->msg = nmsg;
                    }
                    else
                    {
                        nmsg->next=&smoke->head;
                        nmsg = nmsg->next;   // 下一个数据
                    }
                    nmsg->next = NULL;
                }
                break;
            // 0x03-0x7F  预留
            // 0x81~0xFE  用户自定义
            default:
                break;
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
#endif
static int decode_msg_userdef(struct shanghai_userdef *msg, const uint8_t data[], const uint16_t _size)
{
    msg->data = data;
    msg->_dsize = _size;
    return _size;
}

/**
 * 通用解包函数,根据接收数据中的 msg_id 号调用相应子函数解析消息中的数据
 */
static int decode_pack_general(struct obd_agree_obj* const _obd_fops, const uint8_t data[], const uint16_t _dsize, void* const msg_buf, const uint16_t _msize)
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
    //pr_debug("decode_pack_general _dsize:%d | %d \n\n", _dsize, (_pack->data_len+index+1)); fflush(stdout);
    if(_dsize<(_pack->data_len+index+1)) return  ERR_DECODE_PACKL; // 包长度不够
    //pr_debug("decode_pack_general _msize:%d | %d \n", _msize, (_pack->data_len)); fflush(stdout);
    if(_msize<(_pack->data_len)) return  ERR_DECODE_PACKL;         // 数据缓存长度不够
    //pr_debug("decode_pack_general switch %d \n", index); fflush(stdout);
    // 倒数第 1  校验码  BYTE 采用 BCC（异或校验）法，校验范围聪明星单元的第一个字节开始，同后一个字节异或，直到校验码前一字节为止，校验码占用一个字节
    //bcc = BCC_check_code(&data[2], index+_pack->data_len-1);
    bcc = BCC_check_code(&data[2], index+_pack->data_len-2);  // start: 2 byte
    _pack->BCC = data[index+_pack->data_len];
    if(bcc != _pack->BCC) return ERR_DECODE_PACKBCC;                // BCC 校验错误
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
//#ifdef DECODE_SERVER
        case CMD_LOGIN_YJ:          // 车辆登入  上行
            pr_debug("decode_pack_general CMD_LOGIN_YJ \n"); fflush(stdout);
            msg_len = decode_msg_login((struct shanghai_login *)msg_buf, pdata, data_len);
            break;
        case CMD_REPORT_REAL_YJ:    // 实时信息上报  上行
            pr_debug("decode_pack_general CMD_REPORT_REAL_YJ \n"); fflush(stdout);
            msg_len = decode_msg_report_real(msg_buf, _msize, pdata, data_len);
//            pr_debug("save_MSG_OBD VIN: %s \n", save_obd->VIN);
//            pr_debug("save_MSG_OBD SVIN: %s \n", save_obd->SVIN);
//            pr_debug("save_MSG_OBD CVN: %s \n", save_obd->CVN);
            break;
        case CMD_REPORT_LATER_YJ:   // 补发信息上报  上行
            pr_debug("decode_pack_general CMD_REPORT_LATER_YJ \n"); fflush(stdout);
            msg_len = decode_msg_report_real(msg_buf, _msize, pdata, data_len);
            break;
        case CMD_LOGOUT_YJ:         // 车辆登出  上行
            pr_debug("decode_pack_general CMD_LOGOUT_YJ \n"); fflush(stdout);
            msg_len = decode_msg_logout((struct shanghai_logout *)msg_buf, pdata, data_len);
            break;
        case CMD_UTC_YJ:            // 终端校时  上行
            pr_debug("decode_pack_general CMD_UTC_YJ \n"); fflush(stdout);
            msg_len = 0;
            break;
        case CMD_UDE_REAL_YJ:    // 0x82 包含烟雾实时信息上报
            pr_debug("decode_pack_general CMD_UDE_REAL_YJ \n"); fflush(stdout);
            msg_len = decode_msg_report_real(msg_buf, _msize, pdata, data_len);
            break;
        case CMD_UDE_LATER_YJ:   // 0x83 包含烟雾补发信息上报
            pr_debug("decode_pack_general CMD_UDE_LATER_YJ \n"); fflush(stdout);
            msg_len = decode_msg_report_real(msg_buf, _msize, pdata, data_len);
            break;
//#endif
        case CMD_USERDEF_YJ:      // 用户自定义
            pr_debug("decode_pack_general CMD_USERDEF_YJ \n"); fflush(stdout);
            msg_len = decode_msg_userdef((struct shanghai_userdef *)msg_buf, pdata, data_len);
            break;
        default:
            pr_debug("decode_pack_general default \n"); fflush(stdout);
            break;
    }
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
/**
 * 通用校验包函数
 */
//    int (*const check_pack)(struct obd_agree_obj* const _obd_fops, const void* const _data, const uint16_t _dsize);
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
/*
 *A2  建立连接
 * 车载终端向中心平台发起通信连接请求，当通信链路连接建立后，车载终端应自动向发
 * 送登入信息进行身份识别，远程服务与中心平台应对接收到的数据进行校验；校验正确时，
 * 中心平台接收数据；校验错误时，平台应忽略所接收数据。
 */
#if 0
static struct shanghai_login login_pack;
static int handle_request_login(const struct shanghai_login *const request, struct general_pack_shanghai *const _pack)
{
    // ;
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
    return 0;
}
static int handle_request_logout(const struct shanghai_logout *const msg, struct general_pack_shanghai *const _pack)
{
    (void)_pack;
    if(msg->count != login_pack.count) return -1;
    pr_debug("Logout : %s \n", msg->UTC);
    fflush(stdout);
    return 0;
}
static int handle_report_real(const struct shanghai_report_real *const msg, struct general_pack_shanghai *const _pack)
{
    (void)_pack;
//    uint16_t index=0;
//    uint16_t data_len=0;
    struct report_head* nmsg=NULL;  // next msg
    const struct report_head* fault=NULL;
    const struct shanghai_data_obd *obd=NULL;
#if debug_log
    const struct shanghai_data_stream *stream=NULL;
    const struct shanghai_data_att *att=NULL;
#endif

    pr_debug("Report [%d]: %s \n", msg->count, msg->UTC);
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
                while(NULL!=fault)
                {
                    pr_debug("MSG_OBD fault: %d \n", fault->data);  // 故障码, BYTE（4）
                    fault = fault->next;  // 下一个数据
                }
                fflush(stdout);
                break;
            // 2）数据流信息数据格式和定义见表 A.7 所示，补充数据流信息数据格式和定义见表 A.8 所示。
            case MSG_STREAM:     // 0x02  数据流信息
#if debug_log
                stream = (const struct shanghai_data_stream *)container_of(nmsg, struct shanghai_data_stream, head);
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
#endif
                break;
            case MSG_STREAM_ATT: // 0x80  补充数据流
#if debug_log
                att = (const struct shanghai_data_att *)container_of(nmsg, struct shanghai_data_att, head);
                pr_debug("MSG_STREAM_ATT Nm_mode: %d \n", att->Nm_mode);
                pr_debug("MSG_STREAM_ATT accelerator: %d \n", att->accelerator);
                pr_debug("MSG_STREAM_ATT oil_consume: %d \n", att->oil_consume);
                pr_debug("MSG_STREAM_ATT urea_tank_temp: %d \n", att->urea_tank_temp);
                pr_debug("MSG_STREAM_ATT mlh_urea_actual: %d \n", att->mlh_urea_actual);
                pr_debug("MSG_STREAM_ATT mlh_urea_total: %d \n", att->mlh_urea_total);
                pr_debug("MSG_STREAM_ATT exit_gas_temp: %d \n", att->exit_gas_temp);
                fflush(stdout);
#endif
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
#endif
//static struct general_pack_shanghai general_pack_de;
//static struct general_pack_shanghai general_pack_en;
// 初始化函数
static void init(struct obd_agree_obj* const _obd_fops, const uint16_t count, const uint8_t IMEI[16+1], const uint8_t VIN[], const uint16_t soft_version, const char *ssl)
{
//    cJSON *_root = NULL;
//    cJSON *item = NULL;
//    cJSON *info = NULL;
//    cJSON *data = NULL;
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
    struct general_pack_shanghai* pack = &_obd_fops->_gen_pack_YJ;
    (void)count;
    (void)IMEI;
    //read_att(SN);
    memcpy(SN, "440303ZA0CJ20N000020190909", 18);
    SN[18] = '\0';
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
    //memcpy(pack->VIN, "VINTT23456789ABCDEF", sizeof (pack->VIN)-1);
    memcpy(pack->VIN, VIN, sizeof (pack->VIN)-1);
    //memcpy(&general_pack_de, pack, sizeof (general_pack_de));
#if 0
    // JSON
    int ids[4] = { 116, 943, 234, 38793 };
    _root = cJSON_CreateObject();
    cJSON_AddStringToObject(_root, "cJSON Version", cJSON_Version());
    cJSON_AddItemToObject(_root, "Info", info = cJSON_CreateObject());
    cJSON_AddItemToObject(info, "Describe", cJSON_CreateString("This section stores fixed information"));
    cJSON_AddItemToObject(info, "Protocol", cJSON_CreateString("OBD 诊断协议"));
    cJSON_AddItemToObject(info, "MIL", cJSON_CreateString("MIL 状态"));
    cJSON_AddItemToObject(_root, "Data", data = cJSON_CreateObject());
    cJSON_AddItemToObject(data, "Describe", cJSON_CreateString("This section stores dynamic data"));
    cJSON_AddItemToObject(data, "IDs", cJSON_CreateIntArray(ids, 4));

    char *out = NULL;
    out = cJSON_Print(_root);
    printf("%s\n", out);
    mem_free(out);
    ids[0] = 0;
    item = cJSON_CreateIntArray(ids, 4);
    cJSON_ReplaceItemInObject(data, "IDs", item);
    item = cJSON_CreateNumber(2);
    cJSON_ReplaceItemInObject(info, "Protocol", item);
    out = cJSON_Print(_root);
    printf("%s\n", out);
    mem_free(out);
    cJSON_Delete(_root);
#endif
    rsa_int();
}
/*static enum protocol_type protocol(void)
{
    return PRO_TYPE_YJ;
}*/
// 解码包函数
static int handle_decode(struct obd_agree_obj* const _obd_fops, const uint8_t _pack[], const uint16_t _psize, void* const _msg_buf, const uint16_t _msize)
{
#if 0//debug_log
    const char* ssl_type[] = {
        "0x00: NULL",
        "0x01: INFO",
        "0x02: RSA",
        "0x03: SM2",
        "“0xFE”标识异常，“0xFF”表示无效，其他预留 ",
    };
#endif
    //memset(&general_pack_de, 0, sizeof (general_pack_de));
    //struct general_pack_shanghai *const pack = &general_pack_de;
    //struct general_pack_shanghai *const pack = (struct general_pack_shanghai *)_msg_buf;
    struct general_pack_shanghai *const pack = &_obd_fops->_gen_pack_YJ;
    int head_len = sizeof (struct general_pack_shanghai);
    int pack_len = 0;
    //uint8_t msg_buf[1024];
    //uint8_t ack[1024];
    memset(pack, 0, sizeof (struct general_pack_shanghai));
    pack->data = ((char*)_msg_buf)+head_len;//&_msg_buf[head_len];
    //pack->data = &msg_buf[0];
    pack_len = decode_pack_general(_obd_fops, _pack, _psize, pack->data, _msize-head_len);
    //pack_len = decode_pack_general(_pack, _psize, pack, pack->data, 1024);
    //pack_len = decode_pack_general(_pack, _psize, pack, msg_buf, sizeof (msg_buf));
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
        case CMD_LOGIN_YJ:        // 车辆登入
            pr_debug("CMD_LOGIN_YJ\n"); //fflush(stdout);
            //pack_len = handle_request_login((const struct shanghai_login *const)msg_buf, pack);
            pack_len = handle_request_login((const struct shanghai_login *const)pack->data, pack);
            break;
        case CMD_REPORT_REAL_YJ:  // 实时信息上报
            pr_debug("CMD_REPORT_REAL_YJ\n"); //fflush(stdout);
            //handle_report_real((const struct shanghai_report_real *const)msg_buf, pack);
            pack_len = handle_report_real((const struct shanghai_report_real *const)pack->data, pack);
            break;
        case CMD_REPORT_LATER_YJ: // 补发信息上报
            pr_debug("CMD_REPORT_LATER_YJ\n"); //fflush(stdout);
            //handle_report_real((const struct shanghai_report_real *const)msg_buf, pack);
            pack_len = handle_report_real((const struct shanghai_report_real *const)pack->data, pack);
            break;
        case CMD_LOGOUT_YJ:       // 车辆登出
            pr_debug("CMD_LOGOUT_YJ\n"); //fflush(stdout);
            //pack_len = handle_request_logout((const struct shanghai_logout *const)msg_buf, pack);
            pack_len = handle_request_logout((const struct shanghai_logout *const)pack->data, pack);
            break;
        case CMD_UTC_YJ:          // 终端校时
            pr_debug("CMD_UTC_YJ\n"); //fflush(stdout);
            pack_len = 0;
            break;
        case CMD_USERDEF_YJ:      // 用户自定义
            pr_debug("CMD_USERDEF_YJ\n"); //fflush(stdout);
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

static int encode_msg_login(struct obd_agree_obj* const _obd_fops, const struct yunjing_login *const msg, uint8_t buf[], const uint16_t _size)
{
    uint16_t index=0;
    uint16_t data_len=0;
    data_len = sizeof (msg->UTC)-1 + sizeof (msg->count) + sizeof (msg->sn)-1;
    if(data_len>_size) return ERR_ENCODE_PACKL;
    //memset(buf, 0, _size);
    index=0;
    memcpy(&buf[index], msg->UTC, sizeof (msg->UTC)-1);     // 0  数据采集时间  BYTE[6]  时间定义见 A4.4
    BUILD_BUG_ON(sizeof (msg->UTC)-1 != 6);
    index += 6;
    index += bigw_16bit(&buf[index], msg->count);           // 6  登入流水号  WORD 车载终端每登入一次，登入流水号自动加 1，从 1开始循环累加，最大值为 65531，循环周期为天。
    memcpy(&buf[index], msg->sn, sizeof (msg->sn)-1);       // 8  设备序列号由 6 位固定编码和 12 位厂家自定义序号(不重复)共 18 位组成。详情看附件设备序列号要求
    BUILD_BUG_ON(sizeof (msg->sn)-1 != 18);                                               // 倒数第 1  String     1 终止符,固定 ASCII 码“~”，用 0x7E 表示
    index += 18;
    return index; // len
}
static int encode_msg_logout(struct obd_agree_obj* const _obd_fops, const struct yunjing_logout *const msg, uint8_t buf[], const uint16_t _size)
{
    uint16_t index=0;
    uint16_t data_len=0;
    data_len = sizeof (msg->UTC)-1 + sizeof (msg->count) + sizeof (msg->sn)-1;
    if(data_len>_size) return ERR_ENCODE_PACKL;
    //memset(buf, 0, _size);
    index=0;
    memcpy(&buf[index], msg->UTC, sizeof (msg->UTC)-1);     // 0  数据采集时间  BYTE[6]  时间定义见 A4.4
    BUILD_BUG_ON(sizeof (msg->UTC)-1 != 6);
    index += sizeof (msg->UTC)-1;
    index += bigw_16bit(&buf[index], msg->count);           // 6  登入流水号  WORD 车载终端每登入一次，登入流水号自动加 1，从 1开始循环累加，最大值为 65531，循环周期为天。
    memcpy(&buf[index], msg->sn, sizeof (msg->sn)-1);       // 8  设备序列号由 6 位固定编码和 12 位厂家自定义序号(不重复)共 18 位组成。详情看附件设备序列号要求
    BUILD_BUG_ON(sizeof (msg->sn)-1 != 18);                                               // 倒数第 1  String     1 终止符,固定 ASCII 码“~”，用 0x7E 表示
    index += 18;
    return index; // len
}


// A4.5.2  实时信息上报
static int encode_msg_report_real(struct obd_agree_obj* const _obd_fops, const struct shanghai_report_real *const msg, uint8_t buf[], const uint16_t _size)
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
                pr_debug("MSG_OBD : %d \n", index); fflush(stdout);
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
                pr_debug("MSG_STREAM : %d \n", index); fflush(stdout);
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
                pr_debug("MSG_STREAM_ATT : %d \n", index); fflush(stdout);
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
            case MSG_SMOKE: // 0x81  包含烟雾的数据流信息(自定义)
                {
                    if((index+sizeof (struct yunjing_smoke))>_size) return ERR_ENCODE_PACKL;
                    pr_debug("MSG_SMOKE : %d \n", index); fflush(stdout);
                    //att = (const struct shanghai_data_att *)container_of(nmsg, struct shanghai_data_att, head);
                    struct yunjing_smoke* const smoke = container_of(nmsg, struct yunjing_smoke, head);
                    buf[index++] = nmsg->type_msg;
                    index += bigw_16bit(&buf[index], smoke->temperature);   // 0  烟雾排温, 数据长度：2 btyes, 精度：1℃ /bit,
                    index += bigw_16bit(&buf[index], smoke->fault);         // 2  OBD（烟雾故障码）,数据长度：2 btyes 精度：1/bit偏移量：0数据范围：“0xFF，0xFF”表示无效
                    index += bigw_16bit(&buf[index], smoke->kpa);           // 4  背压, 数据长度：2 btyes, 精度：1 kpa/bit,
                    index += bigw_16bit(&buf[index], smoke->m_l);           // 6  光吸收系数,数据长度：2 btyes,精度：0.01 m-l/bit
                    index += bigw_16bit(&buf[index], smoke->opacity);       // 8  不透光度,数据长度：2 btyes,精度：0.1%/bit
                    index += bigw_16bit(&buf[index], smoke->mg_per_m3);     // 10 颗粒物浓度,数据长度：2 btyes,精度：0.1mg/m3 /bit
                    index += bigw_16bit(&buf[index], smoke->light_alarm);   // 12 光吸收系数超标报警,数据长度：2 btyes,精度：1
                    index += bigw_16bit(&buf[index], smoke->pressure_alarm);// 14 背压报警,数据长度：2 btyes,精度：1
                    index += bigw_16bit(&buf[index], smoke->ppm);           // 16 N0x 值,数据长度：2 btyes,精度：1ppm
                }
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
static int encode_msg_report_later(struct obd_agree_obj* const _obd_fops, const struct shanghai_report_real *const msg, uint8_t buf[], const uint16_t _size)
{
    return encode_msg_report_real(_obd_fops, msg, buf, _size);
}
// 自定义数据
static int encode_msg_userdef(struct obd_agree_obj* const _obd_fops, const struct shanghai_userdef *const msg, uint8_t buf[], const uint16_t _size)
{
    if(msg->_dsize>_size) return ERR_ENCODE_PACKL;
    memcpy(buf, msg->data, msg->_dsize);
    return msg->_dsize;
}

/**
 * 通用编码函数,根据参数 msg_id 号调用相应子函数编码消息中的数据
 */
static int encode_pack_general(const enum general_pack_type _pack_type, struct obd_agree_obj* const _obd_fops, const void *const msg, uint8_t buf[], const uint16_t _size)
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
        case CMD_LOGIN_YJ:        // 车辆登入
            data_len = encode_msg_login(_obd_fops, (const struct yunjing_login *)msg, &buf[index], _size-index-1);
            break;
        case CMD_REPORT_REAL_YJ:  // 实时信息上报
            data_len = encode_msg_report_real(_obd_fops, (const struct shanghai_report_real *)msg, &buf[index], _size-index-1);
            break;
        case CMD_REPORT_LATER_YJ: // 补发信息上报
            data_len = encode_msg_report_later(_obd_fops, (const struct shanghai_report_real *)msg, &buf[index], _size-index-1);
            break;
        case CMD_LOGOUT_YJ:       // 车辆登出
            data_len = encode_msg_logout(_obd_fops, (const struct yunjing_logout *)msg, &buf[index], _size-index-1);
            break;
        case CMD_UTC_YJ:          // 终端校时
            data_len = 0;      // 车载终端校时的数据单元为空。
            break;
        case CMD_USERDEF_YJ:      // 用户自定义
            data_len = encode_msg_userdef(_obd_fops, (const struct shanghai_userdef *)msg, &buf[index], _size-index-1);
            break;
        case CMD_UDE_REAL_YJ:    // 0x82 包含烟雾实时信息上报
            data_len = encode_msg_report_real(_obd_fops, (const struct shanghai_report_real *)msg, &buf[index], _size-index-1);
            break;
        case CMD_UDE_LATER_YJ:   // 0x83 包含烟雾补发信息上报
            data_len = encode_msg_report_later(_obd_fops, (const struct shanghai_report_real *)msg, &buf[index], _size-index-1);
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
#include "DateTime.h"
/*void UTC2hhmmss(const uint32_t times, uint8_t buf[], const size_t _size)
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
    buf[1] = localtime.month%13;
    buf[2] = localtime.day%32;
    buf[3] = localtime.hour%24;
    buf[4] = localtime.minute%60;
    buf[5] = localtime.second%60;
}*/
#if 0
static int login(struct obd_agree_obj* const _obd_fops, const uint32_t UTC, const uint16_t count, const uint8_t ICCID[20+1], const uint8_t VIN[17+1], const char* att, uint8_t buf[], const uint16_t _size)
{
    struct yunjing_login msg;
    // 该协议不需要这两个数据
    (void) VIN;
    (void) att;
    memset(&msg, 0, sizeof (msg));
    //msg.UTC[0] = UTC&0xFF;   // 先简单测试
    UTC2hhmmss(UTC, msg.UTC, sizeof (msg.UTC));
    msg.count = count;
    memcpy(msg.sn, SN, sizeof (msg.sn)-1);
    // cmd
    _obd_fops->_gen_pack_YJ.cmd = CMD_LOGIN_YJ;
    // encode
    return encode_pack_general(_obd_fops, &msg, buf, _size);
}
static int logout(struct obd_agree_obj* const _obd_fops, const uint32_t UTC, const uint16_t count, uint8_t buf[], const uint16_t _size)
{
    struct yunjing_logout msg;
    memset(&msg, 0, sizeof (msg));
    //msg.UTC[0] = UTC&0xFF;   // 先简单测试
    UTC2hhmmss(UTC, msg.UTC, sizeof (msg.UTC));
    msg.count = count;
    memcpy(msg.sn, SN, sizeof (msg.sn)-1);
    // cmd
    _obd_fops->_gen_pack_YJ.cmd = CMD_LOGOUT_YJ;
    // encode
    return encode_pack_general(_obd_fops, &msg, buf, _size);
}
// 校时
static int utc(struct obd_agree_obj* const _obd_fops, uint8_t buf[], const uint16_t _size)
{
    /*
     * A4.5.4  终端校时
     *         车载终端校时的数据单元为空。
     */
    // cmd
    _obd_fops->_gen_pack_YJ.cmd = CMD_UTC_YJ;
    // encode
    return encode_pack_general(_obd_fops, NULL, buf, _size);
}

// A4.5.2  实时信息上报
static int report(struct obd_agree_obj* const _obd_fops, const void *const msg, uint8_t buf[], const uint16_t _size)
{
    //struct shanghai_report_real *msg = report_msg(_obd, report_msg_buf, sizeof(report_msg_buf));
    // cmd
    _obd_fops->_gen_pack_YJ.cmd = CMD_REPORT_REAL_YJ;
    // encode
    return encode_pack_general(_obd_fops, msg, buf, _size);
}
// 补发上报
static int report_later(struct obd_agree_obj* const _obd_fops, const void *const msg, uint8_t buf[], const uint16_t _size)
{
    //struct shanghai_report_real *msg = report_msg(_obd, report_msg_buf, sizeof(report_msg_buf));
    // cmd
    _obd_fops->_gen_pack_YJ.cmd = CMD_REPORT_LATER_YJ; // 补发信息上报
    // encode
    return encode_pack_general(_obd_fops, msg, buf, _size);
}
// 编码自定义数据
static int handle_encode(struct obd_agree_obj* const _obd_fops, const void* const data, const uint16_t _dsize, uint8_t pack[], const uint16_t _psize)
{
    const struct shanghai_userdef msg = {data, _dsize};
    // cmd
    _obd_fops->_gen_pack_YJ.cmd = CMD_USERDEF_YJ;
    // encode
    return encode_pack_general(_obd_fops, &msg, pack, _psize);
}
#endif
static int userdef_encode_yj(struct obd_agree_obj* const _obd_fops, const struct yunjing_userdef *const _udef, void* const _buffer, const uint16_t _size)
{
    uint16_t index;
    const struct userdef_yj_qure* const _qure = (const struct userdef_yj_qure*)_udef->msg;
    const struct userdef_yj_qure_ack* const _qure_ack = (const struct userdef_yj_qure_ack*)_udef->msg;
    uint8_t* const buffer = (uint8_t* const)_buffer;
    index=0;
    memset(_buffer, 0, _size);
    index += bigw_16bit(&buffer[index], _udef->count+1); // 以天为单位，每包实时信息流水号唯一，从 1 开始累加
    buffer[index++] = _udef->type_msg;
    switch(_udef->type_msg)
    {
        case USERDEF_YJ_QUERY_VIN:   // 请求 VIN 码
            memcpy(&buffer[index], _udef->msg, 18); index += 18; // 18位SN号
            break;
        case USERDEF_YJ_ACK_VIN:    // 下发 VIN 码
            memcpy(&buffer[index], _udef->msg, 17); index += 17; // 17位VIN码
            break;
        case USERDEF_YJ_QUERY_TIME:  // 请求校时
            ; // 校时数据体为空
            break;
        case USERDEF_YJ_ACK_TIME:   // 下发校时
            memcpy(&buffer[index], _udef->msg, 6); index += 6; // 6位GMT+8 时间,年月日时分秒
            break;
        case USERDEF_YJ_DEV_FAULT:   // 设备故障
            {
                uint8_t count;
                struct userdef_yj_fault* const fault = (struct userdef_yj_fault*)_udef->msg;
                buffer[index++] = fault->sum;
                for(count=0; count<fault->sum; count++)
                {
                    index += bigw_16bit(&buffer[index], fault->value[count]);
                }
            }
            break;
        case USERDEF_YJ_DEV_OFFLINE: // 设备离线
            ; // 设备离线指令数据体为空
            break;
        case USERDEF_YJ_QUERY_CFG:   // 查询配置文件更新
            memcpy(&buffer[index], _qure->sn, 18); index+=18;
            index += bigw_32bit(&buffer[index], _qure->checksum);
            break;
        case USERDEF_YJ_ACK_CFG:     // 响应
            memcpy(&buffer[index], _qure_ack->key, 32); index+=32;
            index += bigw_32bit(&buffer[index], _qure_ack->checksum);
            index += bigw_32bit(&buffer[index], _qure_ack->_total);
            break;
        case USERDEF_YJ_QUERY_FW:    // 查询固件更新
            memcpy(&buffer[index], _qure->sn, 18); index+=18;
            index += bigw_32bit(&buffer[index], _qure->checksum);
            break;
        case USERDEF_YJ_ACK_FW:      // 响应
            memcpy(&buffer[index], _qure_ack->key, 32); index+=32;
            index += bigw_32bit(&buffer[index], _qure_ack->checksum);
            index += bigw_32bit(&buffer[index], _qure_ack->_total);
            break;
        case USERDEF_YJ_QUERY_FWB:    // 查询固件更新,获取固件块信息
            memcpy(&buffer[index], _qure->sn, 18); index+=18;
            index += bigw_32bit(&buffer[index], _qure->checksum);
            break;
        case USERDEF_YJ_ACK_FWB:      // 响应
            {
                const struct userdef_yj_qureb_ack* const _qureb_ack = (const struct userdef_yj_qureb_ack*)_udef->msg;
                memcpy(&buffer[index], _qureb_ack->key, 32); index+=32;
                memcpy(&buffer[index], _qureb_ack->map, 32); index+=32;
                index += bigw_32bit(&buffer[index], _qureb_ack->checksum);
                index += bigw_32bit(&buffer[index], _qureb_ack->_total);
                //index += bigw_16bit(&buffer[index], _qureb_ack->block);
                buffer[index++] = _qureb_ack->value;
                buffer[index++] = _qureb_ack->block;
            }
            break;
        case USERDEF_YJ_DOWNLOAD:     // 下载
            {
                const struct userdef_yj_download* const _download = (const struct userdef_yj_download*)_udef->msg;
                memcpy(&buffer[index], _download->key, 32); index+=32;
                index += bigw_32bit(&buffer[index], _download->_seek);
                index += bigw_32bit(&buffer[index], _download->_total);
                index += bigw_16bit(&buffer[index], _download->block);
            }
            break;
        case USERDEF_YJ_ACK_DOWNLOAD: // 响应
            {
                const struct userdef_yj_ack_download* const _ack_download = (const struct userdef_yj_ack_download*)_udef->msg;
                index += bigw_32bit(&buffer[index], _ack_download->_seek);
                index += bigw_32bit(&buffer[index], _ack_download->_total);
                index += bigw_16bit(&buffer[index], _ack_download->block);
                memcpy(&buffer[index], _ack_download->data, _ack_download->block); index+=_ack_download->block;
            }
            break;
        case USERDEF_YJ_PUSH:         // 推送
            {
                const struct userdef_yj_push* const _push = (const struct userdef_yj_push*)_udef->msg;
                index += bigw_32bit(&buffer[index], _push->checksum_cfg);
                index += bigw_32bit(&buffer[index], _push->checksum_fw);
            }
            break;
        default:
            break;
    }
    return index;
}

static int userdef_decode_yj(struct obd_agree_obj* const _obd_fops, struct yunjing_userdef *const _udef, const void* const _data, const uint16_t _size)
{
    uint16_t index;
    struct userdef_yj_qure* const _qure = (struct userdef_yj_qure*)_udef->msg;
    struct userdef_yj_qure_ack* const _qure_ack = (struct userdef_yj_qure_ack*)_udef->msg;
    const uint8_t* const data = (const uint8_t* const)_data;
    index=0;
    memset(_udef, 0, sizeof(struct yunjing_userdef));
    _udef->count = merge_16bit(data[index], data[index+1]); index+= 2;
    _udef->type_msg = data[index++];
    switch(_udef->type_msg)
    {
        case USERDEF_YJ_QUERY_VIN:   // 请求 VIN 码
            memcpy(_udef->msg, &data[index], 18); index += 18; // 18位SN号
            break;
        case USERDEF_YJ_ACK_VIN:    // 下发 VIN 码
            memcpy(_udef->msg, &data[index], 17); index += 17; // 17位VIN码
            break;
        case USERDEF_YJ_QUERY_TIME:  // 请求校时
            ; // 校时数据体为空
            break;
        case USERDEF_YJ_ACK_TIME:   // 下发校时
            memcpy(_udef->msg, &data[index], 6); index += 6; // 6位GMT+8 时间,年月日时分秒
            break;
        case USERDEF_YJ_DEV_FAULT:   // 设备故障
            {
                uint8_t count;
                struct userdef_yj_fault* const fault = (struct userdef_yj_fault*)_udef->msg;
                fault->sum = data[index++];
                for(count=0; count<fault->sum; count++)
                {
                    fault->value[count] = merge_16bit(data[index], data[index+1]); index+= 2;
                }
            }
            break;
        case USERDEF_YJ_DEV_OFFLINE: // 设备离线
            ; // 设备离线指令数据体为空
            break;
        case USERDEF_YJ_QUERY_CFG:   // 查询配置文件更新
            memcpy(_qure->sn, &data[index], 18); index+=18;
            _qure->checksum = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
            break;
        case USERDEF_YJ_ACK_CFG:     // 响应
            memcpy(_qure_ack->key, &data[index], 32); index+=32;
            _qure_ack->checksum = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
            _qure_ack->_total = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
            break;
        case USERDEF_YJ_QUERY_FW:    // 查询固件更新
            memcpy(_qure->sn, &data[index], 18); index+=18;
            _qure->checksum = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
            break;
        case USERDEF_YJ_ACK_FW:      // 响应
            memcpy(_qure_ack->key, &data[index], 32); index+=32;
            _qure_ack->checksum = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
            _qure_ack->_total = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
            break;
        case USERDEF_YJ_QUERY_FWB:    // 查询固件更新,获取固件块信息
            memcpy(_qure->sn, &data[index], 18); index+=18;
            _qure->checksum = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
            break;
        case USERDEF_YJ_ACK_FWB:      // 响应
            {
                struct userdef_yj_qureb_ack* const _qureb_ack = (struct userdef_yj_qureb_ack*)_udef->msg;
                memcpy(_qureb_ack->key, &data[index], 32); index+=32;
                memcpy(_qureb_ack->map, &data[index], 32); index+=32;
                _qureb_ack->checksum = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
                _qureb_ack->_total = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
                //_qureb_ack->block = merge_16bit(data[index], data[index+1]); index+= 2;
                _qureb_ack->value = data[index++];
                _qureb_ack->block = data[index++];
            }
            break;
        case USERDEF_YJ_DOWNLOAD:     // 下载
            {
                struct userdef_yj_download* const _download = (struct userdef_yj_download*)_udef->msg;
                memcpy(_download->key, &data[index], 32); index+=32;
                _download->_seek = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
                _download->_total = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
                _download->block = merge_16bit(data[index], data[index+1]); index+= 2;
            }
            break;
        case USERDEF_YJ_ACK_DOWNLOAD: // 响应
            {
                struct userdef_yj_ack_download* const _ack_download = (struct userdef_yj_ack_download*)_udef->msg;
                _ack_download->_seek = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
                _ack_download->_total = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
                _ack_download->block = merge_16bit(data[index], data[index+1]); index+= 2;
                memcpy(_ack_download->data, &data[index], _ack_download->block); index+=_ack_download->block;
            }
            break;
        case USERDEF_YJ_PUSH:         // 推送
            {
                struct userdef_yj_push* const _push = (struct userdef_yj_push*)_udef->msg;
                _push->checksum_cfg = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
                _push->checksum_fw = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
            }
            break;
        default:
            break;
    }
    return index;
}

// 自定义编码
static int userdef_encode(struct obd_agree_obj* const _obd_fops, const void *const __udef, void* const _buffer, const uint16_t _size)
{
    return userdef_encode_yj(_obd_fops, (const struct yunjing_userdef *)__udef, _buffer, _size);
}
// 自定义解码
static int userdef_decode(struct obd_agree_obj* const _obd_fops, void *const __udef, const void* const _data, const uint16_t _size)
{
    return userdef_decode_yj(_obd_fops, (struct yunjing_userdef *)__udef, _data, _size);
}
// 更新推送
//static int upload_push(struct obd_agree_obj* const _obd_fops, const uint32_t checksum_cfg, const uint32_t checksum_fw, void * const _buffer, const uint16_t _size)
static int upload_push(struct obd_agree_obj* const _obd_fops, const uint32_t checksum_cfg, const uint32_t checksum_fw, void* const _buffer, const uint16_t _size)
{
    struct yunjing_userdef _udef;
    uint8_t buffer[sizeof(struct yunjing_userdef)];
    int len = 0;
    printf("%s@%d USERDEF_YJ_PUSH\n", __func__, __LINE__);
    struct userdef_yj_push* const _push = (struct userdef_yj_push*)_udef.msg;
    memset(&_udef, 0, sizeof(_udef));
    _udef.type_msg = USERDEF_YJ_PUSH;
    _push->checksum_fw = checksum_fw;
    _push->checksum_cfg = checksum_cfg;
    len = userdef_encode_yj(_obd_fops, &_udef, buffer, sizeof(buffer));
    //return handle_encode(buffer, len, _buffer, _size);
    return _obd_fops->fops->base->pack.encode(_obd_fops, buffer, len, (uint8_t*)_buffer, _size);
}

static const struct obd_agree_fops _obd_agree_fops_yunjing = {
    .constructed = obd_agree_fops_constructed,
    .init = init,
    .protocol = PRO_TYPE_YJ,
    .check_pack = check_pack_general,
    .decode = handle_decode,
//    .encode = handle_encode,
//    .login = login,
//    .logout = logout,
//    .utc = utc,
//    .report = report,
//    .report_later = report_later,
    .userdef_encode = userdef_encode,
    .userdef_decode = userdef_decode,
    .upload_push = upload_push,
    .decode_server = obd_fops_decode_server,
    .decode_client = obd_fops_decode_client,
    .encode_pack_general = encode_pack_general,
    .decode_pack_general = decode_pack_general,
    .protocol_server = obj_obd_agree_yunjing_server,
    .protocol_client = obd_protocol_client_YJ,
    .agree_des = "云景OBD协议",
    //.vin = &_obd_vin_fops,
    .base = &_obd_fops_base,
};
struct obd_agree_obj obd_agree_obj_yunjing = {
    .fops = &_obd_agree_fops_yunjing,
    //.protocol = PRO_TYPE_YJ,
    //._tbuf = "\0",
    //._tlen = 0,
    //._print_buf = "\0",
    ._print = 0,
    ._relay = 0,
    //.protocol_server = obj_obd_agree_yunjing_server,
    //.protocol_client = obd_protocol_client_YJ,
};



