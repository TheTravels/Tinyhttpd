/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : agreement.h
* Author             : Merafour
* Last Modified Date : 11/12/2019
* Description        : OBD agreement.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#ifndef _AGREEMENT_H_
#define _AGREEMENT_H_

#include <stdint.h>
#include "../obd/list.h"
#include "../obd/thread_list.h"

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef __cplusplus

#define bool _Bool
#define true 1
#define false 0

#endif /* !__cplusplus */

#include "diag.h"

 enum protocol_type {
     PRO_TYPE_CCU  = 0x01,  // CCU
     PRO_TYPE_SHH  = 0x02,  // 上海.博泰
     PRO_TYPE_XML  = 0x03,  // XML
     PRO_TYPE_YJ   = 0x04,  // 云景
     PRO_TYPE_NULL,
 };
 // 结构定义
 enum general_pack_type{
     GEN_PACK_LOGIN         = 0x01,       // 0x01  车辆登入  上行
     GEN_PACK_REPORT_REAL   = 0x02,       // 0x02  实时信息上报  上行
     GEN_PACK_REPORT_LATER  = 0x03,       // 0x03  补发信息上报  上行
     GEN_PACK_LOGOUT        = 0x04,       // 0x04  车辆登出  上行
     GEN_PACK_UTC           = 0x05,       // 0x05  终端校时  上行
     GEN_PACK_REV           = 0x06,       // 0x06~0x7F  上行数据系统预留  上行
     GEN_PACK_USERDEF       = 0x81,       // 0x81~0xFE  用户自定义
     GEN_PACK_NULL          = 0x00,       // error
 };
#if 0
 struct agreement_ofp{
    /*
     * 协议初始化函数
     * 参数：初始化参数与相关，具体参数可参考协议登录请求数据包.
     *    count:  消息流水号初值
     *    IMEI:  终端序列号，即：设备 IMEI 号。
     *    soft_version:  终端软件版本号,协议不同版本号德长度不同，单字节版本号中将只保留低字节
     *    ssl:    加密信息，采用文本方式传参，具体包括加密方式与密钥，具体由协议规定，如："AES128,k=1,v=2"(CCU协议),表示使用 AES128加密，密钥组号1，初始化向量组号2.
     */
    void (*init)(const uint16_t count, const uint8_t IMEI[16+1], const uint8_t VIN[], const uint16_t soft_version, const char *ssl);
    // 获取通信协议类型，即可通过该数据识别当前用来与服务器通信的协议
    //enum protocol_type (*protocol)(void);
    const enum protocol_type protocol;
    // 校验包函数
    int (*const check_pack)(const uint8_t data[], const uint16_t _dsize);
    // 解码包函数
    int (*decode)(const uint8_t pack[], const uint16_t _psize, void* const _msg_buf, const uint16_t _msize);
    // 编码自定义数据
    int (*encode)(const void* const data, const uint16_t _dsize, uint8_t pack[], const uint16_t _psize);
    /*
     * 登录验证
     * 参数：登录请求参数与协议的登录数据包相关，具体参数可参考协议登录请求数据包.
     *    UTC:    数据采集时间
     *    count:  登录流水号
     *    ICCID:  SIM 卡的 ICCID 号（集成电路卡识别码）。由 20 位数字的 ASCII 码构成。
     *    VIN:    车辆 VIN 码是识别车辆的唯一标识
     *    att:    附加数据，采用文本方式传参，具体为协议需要的其它数据，具体由协议规定，如："Nm=10"(CCU协议),表示发动机参考扭矩为10(Nm为单位).
     */
    int (*login)(const uint32_t UTC, const uint16_t count, const uint8_t ICCID[20+1], const uint8_t VIN[17+1], const char* att, uint8_t buf[], const uint16_t _size);
    /*
     * 登出
     * UTC:    数据采集时间
     * count:  登出流水号, 登出流水号与当次登入流水号一致。
     */
    int (*logout)(const uint32_t UTC, const uint16_t count, uint8_t buf[], const uint16_t _size);
    // 校时
    int (*utc)(uint8_t buf[], const uint16_t _size);
    // 数据上报
    int (*report)(const void *const msg, uint8_t buf[], const uint16_t _size);
    // 补发上报
    int (*report_later)(const void *const msg, uint8_t buf[], const uint16_t _size);
    // 自定义编码
    int (*userdef_encode)(const void *const __udef, void* const _buffer, const uint16_t _size);
    // 自定义解码
    int (*userdef_decode)(void *const __udef, const void* const _data, const uint16_t _size);
    // 更新推送
    int (*upload_push)(const uint32_t checksum_cfg, const uint32_t checksum_fw, void* const _buffer, const uint16_t _size);
};
#endif
// 单向链表，用于存放信息
struct report_head{
    struct report_head *next;     // 下一条信息
    uint32_t data;                // 可携带四字节数据
    uint8_t type_msg;             // 消息类型
    uint8_t rev[3];
};

//#define  DEVICE_DATA_SIZE   512
//#define  DEVICE_ITEM_SIZE   100
//struct device_item{
//    uint8_t data[DEVICE_DATA_SIZE];      // 数据缓存
//    uint16_t len;
//};
//struct device_data{
//    //enum protocol_type type;
//    struct list_head list;     // 消息链表
//    struct device_item item[DEVICE_ITEM_SIZE];      // 数据缓存
//    uint16_t write;              // 写指针
//    uint16_t read;               // 读指针
//};

// GPS结构
struct report_gps{
    uint32_t UTC;           // 0  DWord  4  UTC+8 时间戳。
    /**
       位 0：定位有效性（0：有效定位、1：无效定位）
       位 1：经度信息（0：东经、1：西经）
       位 2：纬度信息（0：北纬；1：南纬）
       位 3~7：保留
     */
    uint8_t status;        // 4   Byte   1  GPS 的定位状态，每一位的定义如下：
    uint32_t longitude;    // 5   Dword  4  GPS 经度坐标，以度为单位的数值乘以 10 的 6 次方，有正负数。
    uint32_t latitude;     // 9   Dword  4  GPS 纬度坐标，以度为单位的数值乘以 10 的 6 次方，有正负数。
    uint32_t altitude;     // 13  Dword  4  GPS 海拔高度，以米为单位的数值乘以 10，有正负数。
    uint16_t speed;        // 17  Word   2  GPS 速度，以 km/h 为单位的数值乘以 10。
    uint16_t yaw_angle;    // 19  Word   2  GPS 方向，以度为单位的数值乘以 10。
};



#define ERR_DECODE_PACKS         -1      // 包头错误
#define ERR_DECODE_PACKL         -2      // buffer长度错误
#define ERR_DECODE_PACKBCC       -3      // BCC校验错误
#define ERR_DECODE_LOGIN         -4      // 登录包错误
#define ERR_DECODE_LOGOUT        -5      // 登出包错误
#define ERR_DECODE_OBD           -6      // OBD包错误

#define ERR_ENCODE_PACKL         -31     // buffer长度错误

#define ERR_CLIENT_PACKS         -1      // 包错误
#define ERR_CLIENT_CMD           -2      //
#define ERR_CLIENT_DOWN          -31      //
#define STATUS_CLIENT_NULL       0      //
#define STATUS_CLIENT_DOWN       1      //
#define STATUS_CLIENT_DONE       2      //
#define STATUS_CLIENT_PUSH       3      //

// Verify that this architecture packs as expected.
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
/* 取对应字节 */
#define BYTE0(data)    (data&0xFF)
#define BYTE1(data)    ((data>>8)&0xFF)
#define BYTE2(data)    ((data>>16)&0xFF)
#define BYTE3(data)    ((data>>24)&0xFF)

extern uint8_t hexl4(const uint8_t data);
extern uint8_t hexh4(const uint8_t data);
extern uint8_t hex2int(const uint8_t hex);
/* 大端存储 */
extern uint32_t bigw_16bit(uint8_t buf[], const uint16_t data);
extern uint32_t bigw_32bit(uint8_t buf[], const uint32_t data);
extern uint16_t merge_16bit(const uint8_t byte0, const uint8_t byte1);
extern uint32_t merge_32bit(const uint8_t byte0, const uint8_t byte1, const uint8_t byte2, const uint8_t byte3);
// 校验码  倒数第 3  String[2]  2 采用 BCC 异或校验法，由两个 ASCII 码（高位在前，低位在后）组成 8 位校验码。校验范围从接入层协议类型开始到数据单元的最后一个字节。
extern uint8_t BCC_check_code(const uint8_t data[], const uint32_t len);

extern void agreement_test(void);
//extern int decode_server(int* const print, const struct agreement_ofp* _agree_ofp, const uint8_t pack[], const uint16_t _psize, void* const _msg_buf, const uint16_t _msize, struct device_list* const device, void(*csend)(const int sockfd, const void *buf, const uint16_t len), char _buf[], const unsigned int _bsize);
//extern int decode_viewer(int* const print, const struct agreement_ofp* _agree_ofp, const uint8_t pack[], const uint16_t _psize, void* const _msg_buf, const uint16_t _msize, struct device_list* const device);
//extern int decode_client(const struct agreement_ofp* _agree_ofp, const uint8_t pack[], const uint16_t _psize, void* const _msg_buf, const uint16_t _msize, char _tbuf[], const unsigned int _tsize, int* _tlen);
extern const struct agreement_ofp* create_agree_obd_shanghai(void);
extern const struct agreement_ofp *create_agree_obd_yunjing(void);
extern struct shanghai_report_real* shanghai_report_msg(const uint32_t UTC, const OBD_DATA *_obd, const struct report_gps* const _gps, uint8_t _msg_buf[], const uint16_t _msg_size);
extern void set_filter_vin(const char* vin);
extern void set_filter_sn(const char* sn);
extern uint32_t crc_form_file(const char* filename, void* const buffer, const uint32_t bsize);
extern void server_log_init(const uint16_t _port);
extern int server_log_write_to_file(char *__stream, const size_t __n);


// 数据转换
extern float FloatConvert(const int _data, const float offset, const float precision);
extern int IntConvert(const int _data, const float offset, const float precision);
extern double DIntConvert(const uint32_t _data, const double offset, const double precision);
extern void UTC2GMT8(const uint32_t times, uint8_t buf[], const size_t _size);
extern void UTC2hhmmss(const uint32_t times, uint8_t buf[], const size_t _size);
//extern void UTC2Format(const uint32_t times, uint8_t buf[], const size_t _size);

// 下载临时路径
extern const char* download_cfg_temp;
extern const char* download_fw_temp;
extern const char* download_cache_path;
extern const char* config_path;
extern const char* fw_path;
extern const char* Temporary_vin;

#ifdef __cplusplus
}
#endif


#endif // _AGREEMENT_H_
