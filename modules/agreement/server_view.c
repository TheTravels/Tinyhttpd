/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : server_view.c
* Author             : Merafour
* Last Modified Date : 11/21/2019
* Description        : 测试接口，用于获取服务器上接入的设备数据.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#include "server_view.h"
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

// cmd
static enum cmd_unit_shanghai get_cmd(const enum general_pack_type _pack_type)
{
    enum cmd_unit_shanghai _cmd = CMD_NULL;
    switch(_pack_type)
    {
        case GEN_PACK_LOGIN:        // 车辆登入
            _cmd = CMD_VIEW_LOGIN;
            break;
        case GEN_PACK_LOGOUT:       // 车辆登出
            _cmd = CMD_VIEW_LOGOUT;
            break;
        default:
            _cmd = CMD_VIEW_GET_OBD;
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
    (void)_obd_fops;
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
    return  pack_len;
}

// 自定义编码
static int userdef_encode(struct obd_agree_obj* const _obd_fops, const void * const __udef, void * const _buffer, const uint16_t _size)
{
    (void)_obd_fops;
    return upload_encode((const struct upload *)__udef, _buffer, _size);
}
// 自定义解码
static int userdef_decode(struct obd_agree_obj* const _obd_fops, void * const __udef, const void * const _data, const uint16_t _size)
{
    (void)_obd_fops;
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

static const struct obd_agree_fops _obd_agree_fops_view = {
    .constructed = obd_agree_fops_constructed,
    .init = init,
    .protocol = PRO_TYPE_VIEW,
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
struct obd_agree_obj obd_agree_obj_view = {
    .fops = &_obd_agree_fops_view,
    ._print = 0,
    ._relay = 0,
};

