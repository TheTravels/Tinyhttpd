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
#include <string.h>
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
        case GEN_PACK_USERDEF:       // 车辆登出
            _cmd = CMD_VIEW_USERDEF;
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
    struct general_pack_view* const pack = &_obd_fops->_gen_pack_view;
    (void)count;
    (void)IMEI;
    (void)ssl;
    //memset(&login_pack, 0, sizeof (login_pack));
    memset(pack, 0, sizeof (struct general_pack_view));
    // 0  起始符  STRING  固定为 ASCII 字符’##’，用“0x23，0x23”表示
    memcpy(pack->start, "view", 4);
    pack->soft_version = soft_version&0xFF; // 软件版本
    //memcpy(pack->VIN, "VIN0123456789ABCDEF", sizeof (pack->VIN)-1);
    memcpy(pack->VIN, VIN, sizeof (pack->VIN)-1);
    //memcpy(&general_pack_de, pack, sizeof (general_pack_de));
    _obd_fops->_print = 0;
    _obd_fops->_relay = 0;
}
/**
 * 通用校验包函数
 */
static int check_pack_general(struct obd_agree_obj* const _obd_fops, const void* const _data, const uint16_t _dsize)
{
    (void)_obd_fops;
    uint8_t bcc=0;
    uint16_t index=0;
    struct general_pack_view _general_pack;
    struct general_pack_view *const _pack = &_general_pack;
    const uint8_t* const data = (const uint8_t*)_data;
    memset(_pack, 0, sizeof(struct general_pack_view));
    index=0;
    // 0  起始符  STRING  固定为 ASCII 字符’##’，用“0x23，0x23”表示
    _pack->start[0] = data[index++];
    _pack->start[1] = data[index++];
    _pack->start[2] = data[index++];
    _pack->start[3] = data[index++];
    if(0 != strncmp(_pack->start, "view", 4)) return ERR_DECODE_PACKS; // 包头错误
    _pack->cmd = data[index++];                                // 2
    memcpy(_pack->VIN, &data[index], 17);                      // 3
    index += 17;
    memcpy(_pack->sn, &data[index], 18);                      // 3  SN
    index += 18;
    _pack->soft_version = data[index++];                       //  终端软件版本号  BYTE  终端软件版本号有效值范围 0~255
    _pack->protocol = data[index++];
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
    struct general_pack_view *const pack = &_obd_fops->_gen_pack_view;
    int head_len = sizeof (struct general_pack_view);
    int pack_len = 0;
    //printf("[%s-%d] _gen_pack_view \n", __func__, __LINE__);
    memset(pack, 0, sizeof (struct general_pack_view));
    pack->data = ((char*)_msg_buf)+head_len;//&_msg_buf[head_len];
    //printf("@%s-%d \n", __func__, __LINE__);
    pack_len = _obd_fops->fops->decode_pack_general(_obd_fops, _pack, _psize, pack->data, _msize-head_len);
    return  pack_len;
}

static int userdef_encode_view(struct obd_agree_obj* const _obd_fops, const struct general_pack_view_userdef *const _udef, void* const _buffer, const uint16_t _size)
{
    (void)_obd_fops;
    uint16_t index;
    const struct userdef_yj_qure* const _qure = (const struct userdef_yj_qure*)_udef->msg;
    const struct userdef_yj_qure_ack* const _qure_ack = (const struct userdef_yj_qure_ack*)_udef->msg;
    uint8_t* const buffer = (uint8_t* const)_buffer;
    index=0;
    memset(_buffer, 0, _size);
    index += bigw_16bit(&buffer[index], _udef->count+1); // 以天为单位，每包实时信息流水号唯一，从 1 开始累加
    buffer[index++] = _udef->type_msg;
    _obd_fops->_gen_pack_view.userdef = _udef->type_msg;
    switch(_udef->type_msg)
    {
        case USERDEF_VIEW_REQ_OBD:   // 请求 OBD
            memcpy(&buffer[index], _udef->msg, 18); index += 18; // 18位SN号
            break;
        case USERDEF_VIEW_OBD:    // 下发 OBD
            {
                const struct pack_view_udf_obd* const _obd = (struct pack_view_udf_obd*)_udef->msg;
                index += bigw_16bit(&buffer[index], _obd->len);
                memcpy(&buffer[index], _obd->data, _obd->len); index += _obd->len; // OBD Data
            }
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

static int userdef_decode_view(struct obd_agree_obj* const _obd_fops, struct general_pack_view_userdef *const _udef, const void* const _data, const uint16_t _size)
{
    (void)_obd_fops;
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
        case USERDEF_VIEW_REQ_OBD:   // 请求 OBD
            memcpy(_udef->msg, &data[index], 18); index += 18; // 18位SN号
            break;
        case USERDEF_VIEW_OBD:    // 下发 OBD
            {
                struct pack_view_udf_obd* const _obd = (struct pack_view_udf_obd*)_udef->msg;
                _obd->len = merge_16bit(data[index], data[index+1]); index+= 2;
                memcpy(_obd->data, &data[index], _obd->len); index += _obd->len; // OBD Data
            }
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
static int userdef_encode(struct obd_agree_obj* const _obd_fops, const void * const __udef, void * const _buffer, const uint16_t _size)
{
    return userdef_encode_view(_obd_fops, (const struct general_pack_view_userdef *)__udef, _buffer, _size);
}
// 自定义解码
static int userdef_decode(struct obd_agree_obj* const _obd_fops, void * const __udef, const void * const _data, const uint16_t _size)
{
    return userdef_decode_view(_obd_fops, (struct general_pack_view_userdef *)__udef, _data, _size);
}
// 更新推送
static int upload_push(struct obd_agree_obj* const _obd_fops, const uint32_t checksum_cfg, const uint32_t checksum_fw, void * const _buffer, const uint16_t _size)
{
    struct general_pack_view_userdef _udef;
    uint8_t buffer[sizeof(struct general_pack_view_userdef)];
    int len = 0;
    printf("%s@%d USERDEF_YJ_PUSH\n", __func__, __LINE__);
    struct userdef_yj_push* const _push = (struct userdef_yj_push*)_udef.msg;
    memset(&_udef, 0, sizeof(_udef));
    _udef.type_msg = USERDEF_YJ_PUSH;
    _push->checksum_fw = checksum_fw;
    _push->checksum_cfg = checksum_cfg;
    len = userdef_encode_view(_obd_fops, &_udef, buffer, sizeof(buffer));
    return _obd_fops->fops->base->pack.encode(_obd_fops, buffer, len, (uint8_t*)_buffer, _size);
}

static int encode_msg_login(const struct general_pack_view_login *const msg, uint8_t buf[], const uint16_t _size)
{
    uint16_t index=0;
    uint16_t data_len=0;
    data_len = sizeof (msg->UTC)-1 + sizeof (msg->count);
    if(data_len>_size) return ERR_ENCODE_PACKL;
    //memset(buf, 0, _size);
    index=0;
    memcpy(&buf[index], msg->UTC, sizeof (msg->UTC)-1);     // 0  数据采集时间  BYTE[6]  时间定义见 A4.4
    BUILD_BUG_ON(sizeof (msg->UTC)-1 != 6);
    index += 6;
    index += bigw_16bit(&buf[index], msg->count);           // 6  登入流水号  WORD 车载终端每登入一次，登入流水号自动加 1，从 1开始循环累加，最大值为 65531，循环周期为天。
    return index; // len
}
static int encode_msg_logout(const struct general_pack_view_logout *const msg, uint8_t buf[], const uint16_t _size)
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
static int encode_msg_userdef(struct obd_agree_obj* const _obd_fops, const struct shanghai_userdef *const msg, uint8_t buf[], const uint16_t _size)
{
    (void)_obd_fops;
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
    struct general_pack_view * const _pack = &_obd_fops->_gen_pack_view;
    _pack->cmd = get_cmd(_pack_type);
    if(_size<sizeof (struct general_pack_view)) return ERR_ENCODE_PACKL;
    memset(buf, 0, _size);
    index=0;
    pr_debug("encode_pack_general...\n");
    //buf[index++] = _pack->start[0];         // 0  起始符  STRING  固定为 ASCII 字符’##’，用“0x23，0x23”表示
    memcpy(buf, _pack->start, sizeof(_pack->start));
    index += sizeof(_pack->start);
    buf[index++]   = _pack->cmd;              // 2  命令单元  BYTE  命令单元定义见  表 A2  命令单元定义
    memcpy(&buf[index], _pack->VIN, 17);    // 3  车辆识别号  STRING 车辆识别码是识别的唯一标识，由 17 位字码组成，字码应符合 GB16735 中 4.5 的规定
    BUILD_BUG_ON(sizeof (_pack->VIN)-1 != 17);
    index += 17;
    memcpy(&buf[index], _pack->sn, 18);    // 3  车辆识别号  STRING 车辆识别码是识别的唯一标识，由 17 位字码组成，字码应符合 GB16735 中 4.5 的规定
    BUILD_BUG_ON(sizeof (_pack->sn)-1 != 18);
    index += 18;
    buf[index++] = _pack->soft_version;     // 20  终端软件版本号  BYTE  终端软件版本号有效值范围 0~255
    buf[index++] = _pack->protocol;     //
    index += bigw_16bit(&buf[index], _pack->data_len); // 22  数据单元长度  WORD
    // 24  数据单元
    switch(_pack->cmd)
    {
        case CMD_VIEW_LOGIN:        // 车辆登入
            data_len = encode_msg_login((const struct general_pack_view_login *const)msg, &buf[index], _size-index-1);
            break;
        case CMD_VIEW_LOGOUT:       // 车辆登出
            data_len = encode_msg_logout((const struct general_pack_view_logout *const)msg, &buf[index], _size-index-1);
            break;
        case CMD_VIEW_GET_OBD:          // 终端校时
            data_len = 0;      // 车载终端校时的数据单元为空。
            break;
        case CMD_VIEW_USERDEF:      // 用户自定义
            data_len = encode_msg_userdef(_obd_fops, (const struct shanghai_userdef *)msg, &buf[index], _size-index-1);
            break;
        default:
            data_len = 0;
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
static int decode_msg_login(struct general_pack_view_login *msg, const uint8_t data[], const uint16_t _size)
{
    //pthread_mutex_init();
    uint16_t index=0;
    uint16_t data_len = 0;
    index=0;
    pr_debug("decode_msg_login...\n");
    // 数据的总字节长度,  UTC + count + ICCID
    data_len = sizeof (msg->UTC)-1 + sizeof (msg->count) ;
    if(data_len>_size) return ERR_DECODE_LOGIN;    // error
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
static int decode_msg_logout(struct general_pack_view_logout *msg, const uint8_t data[], const uint16_t _size)
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
    const uint8_t* pdata=NULL;
    struct general_pack_view * const _pack = &_obd_fops->_gen_pack_view;
    index=0;
    //printf("[%s-%d] _gen_pack_view \n", __func__, __LINE__);
    memset(msg_buf, 0, _msize);
    _pack->start[0] = data[index++];
    _pack->start[1] = data[index++];
    _pack->start[2] = data[index++];
    _pack->start[3] = data[index++];
    if(0 != strncmp(_pack->start, "view", 4)) return ERR_DECODE_PACKS; // 包头错误
    _pack->cmd = data[index++];                                // 2  命令单元  BYTE  命令单元定义见  表 A2  命令单元定义
    memcpy(_pack->VIN, &data[index], 17);                      // 3  车辆识别号  STRING 车辆识别码是识别的唯一标识，由 17 位字码组成，字码应符合 GB16735 中 4.5 的规定
    index += 17;
    memcpy(_pack->sn, &data[index], 18);                      // 3  SN
    index += 18;
    _pack->soft_version = data[index++];                       // 20  终端软件版本号  BYTE  终端软件版本号有效值范围 0~255
    _pack->protocol = data[index++];
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
    pdata = &data[index];
    // 解码数据
    msg_len = 0;
    switch(_pack->cmd)
    {
        case CMD_VIEW_LOGIN:          // 车辆登入  上行
            pr_debug("decode_pack_general CMD_LOGIN \n"); //fflush(stdout);
            msg_len = decode_msg_login((struct general_pack_view_login *)msg_buf, pdata, data_len);
            break;
        case CMD_VIEW_LOGOUT:         // 车辆登出  上行
            pr_debug("decode_pack_general CMD_LOGOUT \n"); //fflush(stdout);
            msg_len = decode_msg_logout((struct general_pack_view_logout *)msg_buf, pdata, data_len);
            //printf("decode_pack_general CMD_LOGOUT [%d %d]\n", msg_len, data_len);
            break;
        case CMD_VIEW_GET_OBD:            // 终端校时  上行
            pr_debug("decode_pack_general CMD_VIEW_GET_OBD \n"); //fflush(stdout);
            msg_len = 0;
            break;
        case CMD_VIEW_USERDEF:      // 用户自定义
            pr_debug("decode_pack_general CMD_USERDEF \n"); //fflush(stdout);
            msg_len = decode_msg_userdef((struct general_pack_view_userdef *)msg_buf, pdata, data_len);
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
    .protocol_server = obj_obd_agree_general_pack_view_server,
    .protocol_client = obd_protocol_client_view,
    .agree_des = "OBD View协议",
    //.vin = &_obd_vin_fops,
    .base = &_obd_fops_base,
};
struct obd_agree_obj obd_agree_obj_view = {
    .fops = &_obd_agree_fops_view,
    ._print = 0,
    ._relay = 0,
};

