/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : obd_agree_fops.h
* Author             : Merafour
* Last Modified Date : 11/12/2019
* Description        : OBD agreement obj define.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#ifndef OBD_AGREE_FOPS_H_
#define OBD_AGREE_FOPS_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdarg.h>

#include "agreement.h"
#include "obd_agree_shanghai.h"
#include "encrypt.h"
#include "obd_agree_yunjing.h"
#include "server_view.h"
#include "upload.h"
#include "data_base.h"
#include "msg_print.h"

#ifdef	__cplusplus
extern "C" {
#endif

struct obd_agree_ofp_data{
    char _tbuf[2048];
    //const unsigned int _tsize;
    int _tlen;
    //struct data_base_obj* const _db_report;
};

struct obd_agree_obj;
struct obd_vin_fops
{
    int (*const load)(const char *path);
    int (*const search)(const char sn[], char vin[]);
    int (*const insert)(const char* const sn, const char* const vin);
    void (*const req_add)(const char* const sn);
    void (*const req_del)(const char* const sn);
    int (*const req_get)(char _sn[]);
};

struct obd_agree_fops_base{
    struct /*obd_vin_fops*/
    {
        int (*const load)(const char *path);
        int (*const search)(const char sn[], char vin[]);
        int (*const insert)(const char* const sn, const char* const vin);
        void (*const req_add)(const char* const sn);
        void (*const req_del)(const char* const sn);
        int (*const req_get)(char _sn[]);
    }vin;
    struct /*obd_agree_fops_pack*/
    {
        // 编码自定义数据
        int (*const encode)(struct obd_agree_obj* const _obd_fops, const void* const data, const uint16_t _dsize, uint8_t pack[], const uint16_t _psize);
        /*
         * 登录验证
         * 参数：登录请求参数与协议的登录数据包相关，具体参数可参考协议登录请求数据包.
         *    UTC:    数据采集时间
         *    count:  登录流水号
         *    ICCID:  SIM 卡的 ICCID 号（集成电路卡识别码）。由 20 位数字的 ASCII 码构成。
         *    VIN:    车辆 VIN 码是识别车辆的唯一标识
         *    att:    附加数据，采用文本方式传参，具体为协议需要的其它数据，具体由协议规定，如："Nm=10"(CCU协议),表示发动机参考扭矩为10(Nm为单位).
         */
        int (*const login)(struct obd_agree_obj* const _obd_fops, const uint32_t UTC, const uint16_t count, const uint8_t ICCID[20+1], const uint8_t VIN[17+1], const char* att, uint8_t buf[], const uint16_t _size);
        /*
         * 登出
         * UTC:    数据采集时间
         * count:  登出流水号, 登出流水号与当次登入流水号一致。
         */
        int (*const logout)(struct obd_agree_obj* const _obd_fops, const uint32_t UTC, const uint16_t count, uint8_t buf[], const uint16_t _size);
        // 校时
        int (*const utc)(struct obd_agree_obj* const _obd_fops, uint8_t buf[], const uint16_t _size);
        // 数据上报
        int (*const report)(struct obd_agree_obj* const _obd_fops, const void *const msg, uint8_t buf[], const uint16_t _size);
        // 补发上报
        int (*const report_later)(struct obd_agree_obj* const _obd_fops, const void *const msg, uint8_t buf[], const uint16_t _size);
    }pack;
};
struct obd_agree_fops
{
    //int (*const print)(char *__stream, const size_t __n, const char *__format, ...);
    // 构造函数
    struct obd_agree_obj* (*const constructed)(struct obd_agree_obj* const _obd_fops, void* const _obj_fops);
    /*
     * 协议初始化函数
     * 参数：初始化参数与相关，具体参数可参考协议登录请求数据包.
     *    count:  消息流水号初值
     *    IMEI:  终端序列号，即：设备 IMEI 号。
     *    soft_version:  终端软件版本号,协议不同版本号德长度不同，单字节版本号中将只保留低字节
     *    ssl:    加密信息，采用文本方式传参，具体包括加密方式与密钥，具体由协议规定，如："AES128,k=1,v=2"(CCU协议),表示使用 AES128加密，密钥组号1，初始化向量组号2.
     */
    void (*const init)(struct obd_agree_obj* const _obd_fops, const uint16_t count, const uint8_t IMEI[16+1], const uint8_t VIN[], const uint16_t soft_version, const char *ssl);
    // 获取通信协议类型，即可通过该数据识别当前用来与服务器通信的协议
    const enum protocol_type protocol;
    //enum protocol_type protocol(void);
    // 校验包函数
    int (*const check_pack)(struct obd_agree_obj* const _obd_fops, const void* const _data, const uint16_t _dsize);
    // 解码包函数
    int (*const decode)(struct obd_agree_obj* const _obd_fops, const uint8_t pack[], const uint16_t _psize, void* const _msg_buf, const uint16_t _msize);
    // 自定义编码
    int (*const userdef_encode)(struct obd_agree_obj* const _obd_fops, const void *const __udef, void* const _buffer, const uint16_t _size);
    // 自定义解码
    int (*const userdef_decode)(struct obd_agree_obj* const _obd_fops, void *const __udef, const void* const _data, const uint16_t _size);
    // 更新推送
    int (*const upload_push)(struct obd_agree_obj* const _obd_fops, const uint32_t checksum_cfg, const uint32_t checksum_fw, void* const _buffer, const uint16_t _size);

    /**
     * 通用编码函数,根据参数 msg_id 号调用相应子函数编码消息中的数据
     */
    int (*const encode_pack_general)(const enum general_pack_type _pack_type, struct obd_agree_obj* const _obd_fops, const void *const msg, uint8_t buf[], const uint16_t _size);
    /**
     * 通用解包函数,根据接收数据中的 msg_id 号调用相应子函数解析消息中的数据
     */
    int (*const decode_pack_general)(struct obd_agree_obj* const _obd_fops, const uint8_t data[], const uint16_t _dsize, void* const msg_buf, const uint16_t _msize);

    struct /*Father Interface*/
    {
        // server
        int (*const decode_server)(struct obd_agree_obj* const _obd_fops, const uint8_t pack[], const uint16_t _psize, void* const _msg_buf, const uint16_t _msize, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print);
        // client
        int (*const decode_client)(struct obd_agree_obj* const _obd_fops, const uint8_t pack[], const uint16_t _psize, void* const _msg_buf, const uint16_t _msize, struct obd_agree_ofp_data* const _ofp_data, struct msg_print_obj* const _print);
    };
    struct /*Interface*/
    {
        // 服务端接口函数
        int (*const protocol_server)(struct obd_agree_obj* const _obd_fops, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print);
        // 客户端接口函数
        int (*const protocol_client)(struct obd_agree_obj* const _obd_fops, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print);
    };
    const char agree_des[64]; // 协议描述
    //const struct obd_vin_fops* const vin;
    //struct obd_agree_fops_pack* const pack;
    struct obd_agree_fops_base* const base;
};

struct obd_agree_obj
{
    const struct obd_agree_fops* const fops; // 操作函数集合
    // 获取通信协议类型，即可通过该数据识别当前用来与服务器通信的协议
    // const enum protocol_type protocol;
    //char _tbuf[2048];
    //int _tlen;
    //char _print_buf[1024];  //
//protected:
    int _print;
    int _relay;
    // 通用包结构
    union {
        struct general_pack_shanghai _gen_pack;
        struct general_pack_shanghai _gen_pack_YJ;
        struct general_pack_view _gen_pack_view;
    };
    uint32_t fw_crc;  // 固件校验码
    char sn[32];      // 序列号
    char VIN[32];     // 车辆识别码
    char UTC[6];
    //char cache[DEVICE_CACHE_SIZE];      // 数据缓存
    uint16_t fw_update;
    uint16_t fw_flag;
    /**
     * 通用编码函数,根据参数 msg_id 号调用相应子函数编码消息中的数据
     */
    //int encode_pack_general(const void *const msg, struct general_pack_shanghai *const _pack, uint8_t buf[], const uint16_t _size);
    /**
     * 通用解包函数,根据接收数据中的 msg_id 号调用相应子函数解析消息中的数据
     */
    //int decode_pack_general(const uint8_t data[], const uint16_t _dsize, struct general_pack_shanghai *const _pack, void* const msg_buf, const uint16_t _msize);

    //int (*const protocol_server)(struct obd_agree_obj* const _obd_fops, const void* const _general_pack, struct device_list* const device, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print);
    //int (*const protocol_client)(struct obd_agree_obj* const _obd_fops, const void* const _general_pack, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print);
};

extern struct obd_agree_obj* obd_agree_fops_constructed(struct obd_agree_obj* const _obd_fops, void* const _obj_fops);
extern int obd_fops_decode_server(struct obd_agree_obj* const _obd_fops, const uint8_t pack[], const uint16_t _psize, void* const _msg_buf, const uint16_t _msize, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print);
extern int obj_obd_agree_shanghai_server(struct obd_agree_obj* const _obd_fops, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print);
extern int obd_fops_decode_client(struct obd_agree_obj* const _obd_fops, const uint8_t pack[], const uint16_t _psize, void* const _msg_buf, const uint16_t _msize, struct obd_agree_ofp_data* const _ofp_data, struct msg_print_obj* const _print);
extern int obd_protocol_client_shanghai(struct obd_agree_obj* const _obd_fops, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print);

extern int obj_obd_agree_yunjing_server(struct obd_agree_obj* const _obd_fops, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print);
extern int obd_protocol_client_YJ(struct obd_agree_obj* const _obd_fops, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print);

extern int obj_obd_agree_general_pack_view_server(struct obd_agree_obj* const _obd_fops, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print);
extern int obd_protocol_client_view(struct obd_agree_obj* const _obd_fops, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print);

//extern const struct obd_agree_fops _obd_agree_fops_shanghai;
extern struct obd_agree_obj obd_agree_obj_shanghai;
//extern const struct obd_agree_fops _obd_agree_fops_yunjing;
extern struct obd_agree_obj obd_agree_obj_yunjing;
extern struct obd_agree_obj obd_agree_obj_view;

extern struct obd_vin_fops _obd_vin_fops;
extern struct obd_agree_fops_pack _obd_fops_pack;
extern struct obd_agree_fops_base _obd_fops_base;

#ifdef	__cplusplus
}
#endif

#endif // _OBD_AGREE_FOPS_H_
