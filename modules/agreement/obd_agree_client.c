/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : obd_agree_client.c
* Author             : Merafour
* Last Modified Date : 11/12/2019
* Description        : OBD agreement client.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include<fcntl.h>
#include <unistd.h>

#include <time.h>
#include "agreement.h"
#include "obd_agree_shanghai.h"
#include "obd_agree_yunjing.h"
#include "DateTime.h"
#include "upload.h"
#include "obd_agree_fops.h"
#include "obd_agree_shanghai.h"

#define  debug_log     0
#ifndef debug_log
#define  debug_log     1
#endif
//#undef   debug_log
#include <stdio.h>
#include <string.h>

//#ifdef debug_log
#if debug_log
#define pr_debug(fmt, ...) printf(fmt, ##__VA_ARGS__); fflush(stdout);
#else
#define pr_debug(fmt, ...) ; //
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
//const char* download_log="download.log";
//const char* download_log_path="/download.log";
extern const char* download_log;//="download.log";
extern const char* download_log_path;//="/download.log";

#if 0
static long get_file_size(const char* filename)
{
    long _size=0;
    FILE* fd = fopen(filename, "r");
    if(NULL == fd)
    {
        pr_debug("file %s not exist! \n", filename);  fflush(stdout);
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
//static char* load_file(const char* filename, long* const size)
//{
//    static char filebin[2*1024*1024]; // 2MB
//    long _size=0;
//    FILE* fd = fopen(filename, "r");
//    if(NULL == fd)
//    {
//        pr_debug("file %s not exist! \n", filename);  fflush(stdout);
//        return NULL;
//    }
//    else
//    {
//        fseek(fd, 0, SEEK_END);
//        _size = ftell(fd);
//        fseek(fd, 0, SEEK_SET);
//        // read
//        memset(filebin, 0xFF, sizeof (filebin));
//        _size = fread(filebin, (size_t)_size, 1, fd);
//        if(_size<=0) return NULL;
//        *size = _size;
//        return filebin;
//    }
//}
static uint8_t obd_buf[1024*10];

static int save_to_file(const char* const filename, const uint32_t seek, const void* const data, const uint16_t len)
{
    long _size=0;
#if 0
    FILE* fd = fopen(filename, "r");
    if(NULL == fd)
    {
        printf("file %s not exist! \n", filename);  fflush(stdout);
        return NULL;
    }
    else
    {
        fseek(fd, 0, SEEK_END);
        _size = ftell(fd);
        fseek(fd, 0, SEEK_SET);
        // read
        memset(filebin, 0xFF, sizeof (filebin));
        _size = fread(filebin, (size_t)_size, 1, fd);
        if(_size<=0) return NULL;
        *size = _size;
        return filebin;
    }
#else
    FILE* fd = NULL;
    if(0==seek)
    {
        fd = fopen(filename, "w");
        fclose(fd);
    }
    fd = fopen(filename, "ab");
    if(NULL == fd)
    {
        printf("file %s not exist!..2 \n", filename);  fflush(stdout);
        return -1;
    }
    else
    {
        _size = fwrite(data, (size_t)len, 1, fd);
        if(_size<=0) return -2;
        fclose(fd);
    }
#endif
    return 0;
}
#if 0
static int copy_to_file(const char* const filesrc, const char* const filedes)
{
    long _size=0;
    long len=0;
    long _rize=0;
    size_t count=0;
    char filebin[512]; //
    FILE* fds = NULL;
    FILE* fdd = NULL;
    //printf("%s@%d fopen filesrc \n", __func__, __LINE__); fflush(stdout);
    fds = fopen(filesrc, "r");
    if(NULL == fds)
    {
        printf("filesrc %s not exist! \n", filesrc);  fflush(stdout);
        return -1;
    }
    //printf("%s@%d fopen filedes \n", __func__, __LINE__); fflush(stdout);
    fdd = fopen(filedes, "w");
    if(NULL == fdd)
    {
        printf("filedes %s not exist! \n", filedes);  fflush(stdout);
        fclose(fds);
        return -1;
    }
    //printf("%s@%d fseek \n", __func__, __LINE__); fflush(stdout);
    fseek(fds, 0, SEEK_END);
    _size = ftell(fds);
    fseek(fds, 0, SEEK_SET);
    //_size = total;
    _rize = 0;
    //printf("%s@%d for _size:%ld len:%ld _rize:%ld \n", __func__, __LINE__, _size, len, _rize); fflush(stdout);
    for(len=0; len<_size; len+=_rize)
    {
        _rize = _size - len;
        if(_rize>512) _rize=512;
        // read
        memset(filebin, 0xFF, sizeof (filebin));
        count=fread(filebin, (size_t)_rize, 1, fds);
        //printf("%s@%d fread _size:%ld len:%ld _rize:%ld count:%ld \n", __func__, __LINE__, _size, len, _rize, count); fflush(stdout);
        count=fwrite(filebin, (size_t)_rize, 1, fdd);
        //printf("%s@%d fwrite _size:%ld len:%ld _rize:%ld count:%ld \n", __func__, __LINE__, _size, len, _rize, count); fflush(stdout);
    }
    fclose(fds);
    fclose(fdd);
    return 0;
}
#endif

static int handle_request_userdef(struct obd_agree_obj* const _obd_fops, const struct general_pack_shanghai* const _pack, char _tbuf[], const unsigned int _tsize, int* const _tlen)
{
    const struct shanghai_userdef *const msg = (const struct shanghai_userdef *)(_pack->data);
    struct upload decode_buf;
    struct upload* const decode = &decode_buf;
    char filename[128];
    uint8_t buffer[sizeof(struct upload)];
    static uint8_t down_type=0;    // 0:cfg, 1:fw
    uint32_t _size=0;
    const char *suffix=NULL;
    uint32_t checksum;
    static uint32_t server_checksum;
    int len;
    int ret = STATUS_CLIENT_NULL;
    int fd = 0;
    len = upload_decode(decode, msg->data, msg->_dsize);
    ret = STATUS_CLIENT_NULL;
    if(len<0)
    {
        pr_debug("userdef decode error!\n"); fflush(stdout);
    }
    else
    {
//        pr_debug("decode cmd : 0x%02X\n", decode.cmd); fflush(stdout);
//        pr_debug("decode total : %d\n", decode.pack_total); fflush(stdout);
//        pr_debug("decode index : %d\n", decode.pack_index); fflush(stdout);
//        pr_debug("decode checksum : 0x%02X\n", decode.checksum); fflush(stdout);
//        pr_debug("decode Model : %s\n", decode.Model); fflush(stdout);
//        pr_debug("decode len : %d\n", decode.data_len); fflush(stdout);
//        pr_debug("decode data : %s\n", decode.data); fflush(stdout);
//        pr_debug("decode CRC : 0x%02X\n", decode.CRC); fflush(stdout);
        switch (decode->cmd)
        {
            case UPLOAD_LOGIN:  // 登录
                pr_debug("userdef Login\n"); fflush(stdout);
                break;
            case UPLOAD_VIN:
                if(0==decode->data_len)
                {
                    ret = ERR_CLIENT_DOWN;
                }
                else
                {
                    //save_vin(decode->data);
                    ret = STATUS_CLIENT_DONE;
                }
                break;
            case UPLOAD_QUERY_CFG:    // 查询
#if 0
                pr_debug("userdef Config\n"); fflush(stdout);
                pr_debug("userdef Config: total:%d seek:%d\n", decode->pack_total, decode->pack_index); fflush(stdout);
                //pr_debug("userdef filename: %s\n", decode->data); fflush(stdout);
#endif
                if( ((decode->pack_total<<1)==decode->pack_index) && (decode->data_len>strlen("/*.cfg")+1) ) // update
                {
                    server_checksum=decode->checksum;
                    down_type = 0;
                    decode->pack_index = 0;
                    decode->down_len = FRAME_DATA_LEN;
                    if(decode->down_len>decode->pack_total) decode->down_len=decode->pack_total;
                    decode->cmd = UPLOAD_DOWNLOAD;
                    memset(filename, 0, sizeof (filename));
                    memcpy(filename, decode->data, strlen((const char *)decode->data));
                    decode->data_len = strlen((const char *)decode->data)+1;
                    //_load = upload_init(UPLOAD_LOGIN, 0, 0, 0x12345678, "OBD1234567890ABCDEF", "Hello", 5);
                    memset(buffer, 0, sizeof (buffer));
                    len = upload_encode(decode, buffer, sizeof (buffer));   // user def
                    len = _obd_fops->fops->base->pack.encode(_obd_fops, buffer, len, obd_buf, sizeof (obd_buf)); // encode
                    /*if(NULL != csend) // 发送数据到客户端
                    {
                        csend(client, rsa_buffer, len);
                    }*/
                    if((NULL != _tbuf) && (len>0))
                    {
                        memcpy(_tbuf, obd_buf, len);
                        *_tlen = len;
                    }
                    ret = STATUS_CLIENT_DOWN;
                    //mkdir("./upload");
                    //rename(filename, "./upload/Download.dat");
                }
                else  ret = STATUS_CLIENT_DONE;
                break;
            case UPLOAD_DOWNLOAD:   // 下载
                pr_debug("userdef DownLoad: total:%d seek:%d size:%d\n", decode->pack_total, decode->pack_index, decode->data_len); fflush(stdout);
                if( (0!=decode->data_len) && (decode->data_len==decode->down_len) ) // update
                {
                    //save_to_file(filename, decode->pack_index, decode->data, decode->down_len);
                    save_to_file(download_cache_path, decode->pack_index, decode->data, decode->down_len);
                    decode->pack_index = decode->pack_index + decode->down_len;
                    _size = decode->pack_total - decode->pack_index;
                    if(_size>FRAME_DATA_LEN) _size = FRAME_DATA_LEN;
                    if(0==_size)  // done
                    {
                        ret = STATUS_CLIENT_DONE;
                        memset(buffer, 0, sizeof (buffer));
#if 0
                        snprintf((char*)buffer, sizeof (filename)-1, "/%s", filename);
#else                   // cfg
                        if(0==down_type) memcpy(buffer, download_cfg_temp, strlen(download_cfg_temp));
                        // fw
                        else memcpy(buffer, download_fw_temp, strlen(download_fw_temp));
#endif
                        // 匹配校验码
                        server_checksum=decode->checksum;
                        checksum = crc_form_file(download_cache_path, obd_buf, sizeof (obd_buf));
                        //printf("checksum: 0x%08X 0x%08X\n", checksum, server_checksum);
                        if(checksum!=server_checksum)
                        {
                            ret = ERR_CLIENT_DOWN;
                            //break;
                        }
                        fd = open((const char *)buffer, O_RDONLY, 0);
                        if(0 <= fd)
                        {
                            close(fd);
                            //remove(buffer);
                            unlink((const char *)buffer);
                        }
                        rename(download_cache_path, (const char *)buffer);
                        //printf("\n\n %s Download Done!\n", buffer); fflush(stdout);
                    }
                    else
                    {
                        decode->down_len = _size;
                        decode->cmd = UPLOAD_DOWNLOAD;
                        decode->data_len = strlen(filename)+1;
                        memcpy(decode->data, filename, decode->data_len);
                        //_load = upload_init(UPLOAD_LOGIN, 0, 0, 0x12345678, "OBD1234567890ABCDEF", "Hello", 5);
                        memset(buffer, 0, sizeof (buffer));
                        len = upload_encode(decode, buffer, sizeof (buffer));   // user def
                        len = _obd_fops->fops->base->pack.encode(_obd_fops, buffer, len, obd_buf, sizeof (obd_buf)); // encode
                        /*if(NULL != csend)
                        {
                            csend(client, rsa_buffer, len); // 发送数据到客户端
                        }*/
                        if((NULL != _tbuf) && (len>0))
                        {
                            memcpy(_tbuf, obd_buf, len);
                            *_tlen = len;
                        }
                        ret = STATUS_CLIENT_DOWN;
                    }
                }
                else ret = ERR_CLIENT_DOWN;
                break;
            case UPLOAD_QUERY_FW:    // 查询
                //printf("userdef Config\n"); fflush(stdout);
                if( ((decode->pack_total<<1)==decode->pack_index) && (decode->data_len>strlen("/*.bin")+1) ) // update
                {
                    server_checksum=decode->checksum;
                    down_type = 1;
                    decode->pack_index = 0;
                    decode->down_len = FRAME_DATA_LEN;
                    if(decode->down_len>decode->pack_total) decode->down_len=decode->pack_total;
                    decode->cmd = UPLOAD_DOWNLOAD;
                    memset(filename, 0, sizeof (filename));
                    memcpy(filename, decode->data, strlen((const char *)decode->data));
                    decode->data_len = strlen((const char *)decode->data)+1;
                    //_load = upload_init(UPLOAD_LOGIN, 0, 0, 0x12345678, "OBD1234567890ABCDEF", "Hello", 5);
                    memset(buffer, 0, sizeof (buffer));
                    len = upload_encode(decode, buffer, sizeof (buffer));   // user def
                    len = _obd_fops->fops->base->pack.encode(_obd_fops, buffer, len, obd_buf, sizeof (obd_buf)); // encode
                    /*if(NULL != csend)
                    {
                        csend(client, rsa_buffer, len); // 发送数据到客户端
                    }*/
                    if((NULL != _tbuf) && (len>0))
                    {
                        memcpy(_tbuf, obd_buf, len);
                        *_tlen = len;
                    }
                    ret = STATUS_CLIENT_DOWN;
                    //mkdir("./upload");
                    //rename(filename, "./upload/Download.dat");
                }
                else  ret = STATUS_CLIENT_DONE;
                break;
            case UPLOAD_PUSH:   // 推送固件
                printf("userdef Push\n"); fflush(stdout);
                //checksum = crc_form_file(fw_path, buffer, sizeof(buffer) );
                server_checksum=decode->checksum;
                checksum = crc_form_file(fw_path, obd_buf, sizeof (obd_buf));
                printf("Push checksum: 0x%04X 0x%04X \n", server_checksum, checksum); fflush(stdout);
                if(checksum!=server_checksum)
                {
                    ret = STATUS_CLIENT_PUSH;
                }
                break;
            case UPLOAD_GET_TIME:        // 校时
                //if(0==server_sync_time(decode->pack_total)) ret = STATUS_CLIENT_DONE;
                ret = STATUS_CLIENT_DONE;
                break;
            default:
                break;
        }
    }
    return ret;
}
#define DOWNLOAD_TYPE_CFG    0
#define DOWNLOAD_TYPE_FW     1
#define DOWNLOAD_TYPE_FWB    2
static int handle_request_userdef_yunjing(struct obd_agree_obj* const _obd_fops, const struct general_pack_shanghai* const _pack, char _tbuf[], const unsigned int _tsize, int* const _tlen)
{
    const struct shanghai_userdef *const msg = (const struct shanghai_userdef *)(_pack->data);
    static uint8_t _buffer[1024+512];
    struct yunjing_userdef* const _udef = (struct yunjing_userdef*)_buffer;
    int len;
    int ret = STATUS_CLIENT_NULL;
    //len = userdef_decode_yj(_udef, msg->data, msg->_dsize);
    len = _obd_fops->fops->userdef_decode(_obd_fops, _udef, msg->data, msg->_dsize);
    ret = STATUS_CLIENT_NULL;
    if(len<0)
    {
        pr_debug("userdef decode error!\n"); fflush(stdout);
    }
    else
    {
        switch (_udef->type_msg)
        {
            case USERDEF_YJ_ACK_VIN:
                if(0==_udef->msg[0])
                {
                    printf("UPLOAD_VIN ERR\r\n");
                    ret = ERR_CLIENT_DOWN;
                }
                else
                {
                    //printf("UPLOAD_VIN [%s]\r\n", _udef->msg);
                    //save_vin((uint8_t*)_udef->msg);
                    if((NULL != _tbuf) && (len>0) && (_tsize>20))
                    {
                        memset(_tbuf, 0, 20);
                        memcpy(_tbuf, _udef->msg, strlen(_udef->msg));
                        *_tlen = len;
                    }
                    ret = STATUS_CLIENT_DONE;
                }
                break;
            default:
                    break;
        }
    }
    //printf("UPLOAD_VIN [%d]\r\n", ret);
    //printf("%s--%s \n", __func__, __LINE__);
    //printf("%s--%s ret:%d\n", __func__, __LINE__, ret);
    return ret;
}
int obd_protocol_client_shanghai(struct obd_agree_obj* const _obd_fops, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print)
//static int protocol_shanghai(struct obd_agree_obj* const _obd_fops, const struct general_pack_shanghai* const _pack, char _tbuf[], const unsigned int _tsize, int* const _tlen)
{
#if debug_log
    const char* ssl_type[] = {
        "0x00: NULL",
        "0x01：数据不加密",
        "0x02：数据经过 RSA 算法加密",
        "0x03：数据经过国密 SM2 算法加密",
        "“0xFE”标识异常，“0xFF”表示无效，其他预留 ",
    };
#endif
    char* const _tbuf = _ofp_data->_tbuf;
    const unsigned int _tsize = sizeof(_ofp_data->_tbuf);
    int* const _tlen = &_ofp_data->_tlen;
    int ret = 0;
    //const struct general_pack_shanghai* const pack = (const struct general_pack_shanghai*)_general_pack;
    const struct general_pack_shanghai* const pack = &_obd_fops->_gen_pack;
    pr_debug("start: %c%c \n", pack->start[0], pack->start[1]);
    pr_debug("CMD: %d \n", pack->cmd);
    pr_debug("VIM: %s\n", pack->VIN);
    pr_debug("soft_version: %d \n", pack->soft_version);
    pr_debug("ssl: %s \n", ssl_type[pack->ssl&0x3]);
    pr_debug("data_len: %d \n", pack->data_len);
    pr_debug("BCC: %02X \n", pack->BCC);
    //fflush(stdout);
    switch(pack->cmd)
    {
#if 0
        case CMD_LOGIN:        // 车辆登入
            pr_debug("车辆登入\n"); fflush(stdout);
            //pack_len = handle_request_login((const struct shanghai_login *const)msg_buf, pack);
            handle_request_login(pack);
            break;
        case CMD_REPORT_REAL:  // 实时信息上报
            pr_debug("实时信息上报\n"); fflush(stdout);
            //handle_report_real((const struct shanghai_report_real *const)msg_buf, pack);
            handle_report_real(pack);
            break;
        case CMD_REPORT_LATER: // 补发信息上报
            pr_debug("补发信息上报\n"); fflush(stdout);
            //handle_report_real((const struct shanghai_report_real *const)msg_buf, pack);
            handle_report_real(pack);
            break;
        case CMD_LOGOUT:       // 车辆登出
            pr_debug("车辆登出\n"); fflush(stdout);
            //pack_len = handle_request_logout((const struct shanghai_logout *const)msg_buf, pack);
            handle_request_logout(pack);
            break;
        case CMD_UTC:          // 终端校时
            pr_debug("终端校时\n"); fflush(stdout);
            break;
#endif
        case CMD_USERDEF:      // 用户自定义
            pr_debug("CMD_USERDEF\n"); fflush(stdout);
            ret = handle_request_userdef(_obd_fops, pack, _tbuf, _tsize, _tlen);
            break;
        default:
            pr_debug("default\n"); fflush(stdout);
            break;
    }
    return ret;
}
int obd_protocol_client_YJ(struct obd_agree_obj* const _obd_fops, struct obd_agree_ofp_data* const _ofp_data, struct data_base_obj* const _db_report, struct msg_print_obj* const _print)
//static int protocol_yunjing(struct obd_agree_obj* const _obd_fops, const struct general_pack_shanghai* const _pack, char _tbuf[], const unsigned int _tsize, int* const _tlen)
{
#if debug_log
    const char* ssl_type[] = {
        "0x00: NULL",
        "0x01：数据不加密",
        "0x02：数据经过 RSA 算法加密",
        "0x03：数据经过国密 SM2 算法加密",
        "“0xFE”标识异常，“0xFF”表示无效，其他预留 ",
    };
#endif
    char* const _tbuf = _ofp_data->_tbuf;
    const unsigned int _tsize = sizeof(_ofp_data->_tbuf);
    int* const _tlen = &_ofp_data->_tlen;
    int ret = 0;
    //const struct general_pack_shanghai* const pack = (const struct general_pack_shanghai*)_general_pack;
    const struct general_pack_shanghai* const pack = &_obd_fops->_gen_pack;
    pr_debug("start: %c%c \n", pack->start[0], pack->start[1]);
    pr_debug("CMD: %d \n", pack->cmd);
    pr_debug("VIM: %s\n", pack->VIN);
    pr_debug("soft_version: %d \n", pack->soft_version);
    pr_debug("ssl: %s \n", ssl_type[pack->ssl&0x3]);
    pr_debug("data_len: %d \n", pack->data_len);
    pr_debug("BCC: %02X \n", pack->BCC);
    fflush(stdout);
    switch(pack->cmd)
    {
#if 0
        case CMD_LOGIN_YJ:        // 车辆登入
            pr_debug("车辆登入\n"); fflush(stdout);
            //pack_len = handle_request_login((const struct shanghai_login *const)msg_buf, pack);
            handle_request_login(pack);
            break;
        case CMD_REPORT_REAL_YJ:  // 实时信息上报
            pr_debug("实时信息上报\n"); fflush(stdout);
            //handle_report_real((const struct shanghai_report_real *const)msg_buf, pack);
            handle_report_real(pack);
            break;
        case CMD_REPORT_LATER_YJ: // 补发信息上报
            pr_debug("补发信息上报\n"); fflush(stdout);
            //handle_report_real((const struct shanghai_report_real *const)msg_buf, pack);
            handle_report_real(pack);
            break;
        case CMD_LOGOUT_YJ:       // 车辆登出
            pr_debug("车辆登出\n"); fflush(stdout);
            //pack_len = handle_request_logout((const struct shanghai_logout *const)msg_buf, pack);
            handle_request_logout(pack);
            break;
        case CMD_UTC_YJ:          // 终端校时
            pr_debug("终端校时\n"); fflush(stdout);
            break;
#endif
        case CMD_USERDEF_YJ:      // 用户自定义
            pr_debug("CMD_USERDEF_YJ\n"); fflush(stdout);
            ret = handle_request_userdef_yunjing(_obd_fops, pack, _tbuf, _tsize, _tlen);
            break;
        default:
            pr_debug("default\n"); fflush(stdout);
            break;
    }
    //printf("UPLOAD_VIN [%d]\r\n", ret);
    return ret;
}





