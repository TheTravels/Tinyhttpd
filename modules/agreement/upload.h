/************************ (C) COPYLEFT 2018 Merafour *************************
* File Name          : upload.h
* Author             : Merafour
* Last Modified Date : 01/10/2019
* Description        : 数据上载协议，该协议使用其它协议作为底层协议，如协议的自定义部分.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#ifndef UPLOAD_H
#define UPLOAD_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

 enum upload_status{
       STATUS_QUERY_CFG   = 0x00,
       STATUS_UPLOAD_CFG  = 0x01,
       STATUS_QUERY_FW    = 0x02,
       STATUS_UPLOAD_FW   = 0x03,
       STATUS_REBOOT      = 0x04,
       STATUS_JUMP        = 0x05,
       STATUS_ERR         = 0x06,
       STATUS_TIMEOUT     = 0x07,
       STATUS_RUN         = 0x08,
       STATUS_VIN         = 0x09,
       STATUS_GET_TIME    = 0x0A,  // 校时
       STATUS_QUERYB_FW   = 0x0B,  // 块下载
 };

 enum upload_cmd{
     UPLOAD_ACK       = 0x00,  // 应答
     UPLOAD_LOGIN     = 0x01,  // 登录
     UPLOAD_QUERY_CFG = 0x02,  // 查询
     //UPLOAD_DOWN_CFG  = 0x03,  // 下载
     UPLOAD_QUERY_FW  = 0x04,  // 查询
     //UPLOAD_DOWN_FW   = 0x05,  // 下载
     UPLOAD_DOWNLOAD  = 0x05,  // 下载
     UPLOAD_PUSH      = 0x06,  // 推送
     UPLOAD_VIN       = 0x07,  // 从服务器获取 VIN码
     UPLOAD_GET_TIME  = 0x08,  // 校时
     REPORT_VERSION   = 0x09,  // 提交版本信息,上行
 };

 /**
  *  |  1  |   4   |   4   |   4   |   16  |  2   |  2  |  n   |  1  |
  *  | cmd | total | index | check | Model | down | len | data | CRC |
  */
#define FRAME_DATA_LEN       1024
 struct upload{
     uint8_t CRC;
     uint8_t cmd;
     uint32_t pack_total;
     uint32_t pack_index;
     uint32_t checksum;
     uint8_t Model[16];
     uint16_t down_len;
     uint16_t data_len;
     uint8_t data[FRAME_DATA_LEN];     // 最多一次携带 1024 byte数据
 };

 extern struct upload* upload_init(const enum upload_cmd _cmd, const uint32_t pack_total, const uint32_t pack_index, const uint32_t checksum, const char Model[16], const void* const data, const uint16_t data_len);
 extern int upload_encode(const struct upload* const _load, void* const _buffer, const uint16_t _size);
 extern int upload_decode(struct upload* _load, const void* const _data, const uint16_t _size);
 extern int upload_test(void);

 // 计算 CRC
 extern unsigned short fast_crc16(unsigned short sum, const unsigned char *p, unsigned int len);



#ifdef __cplusplus
}
#endif

#endif // UPLOAD_H
