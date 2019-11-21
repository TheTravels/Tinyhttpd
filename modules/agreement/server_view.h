/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : server_view.h
* Author             : Merafour
* Last Modified Date : 11/21/2019
* Description        : 测试接口，用于获取服务器上接入的设备数据.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#ifndef _SERVER_VIEW_H_
#define _SERVER_VIEW_H_

//#include "obd_agree_fops.h"

#include <stdint.h>
#include "agreement.h"

#ifdef	__cplusplus
extern "C" {
#endif

 // 命令单元定义
 enum cmd_unit_view{
     CMD_VIEW_LOGIN    = 0x01,       // 0x01  Client 登入
     CMD_VIEW_LOGOUT   = 0x02,       // 0x02  Client 登出
     CMD_VIEW_GET_OBD  = 0x03,       // 0x03  获取OBD 数据
     CMD_VIEW_EMPTY    = 0x04,       // 0x04  empty
     CMD_VIEW_NULL          = 0x00,       // error
 };

 // 通用包结构
 struct general_pack_view{
     uint8_t start[4];     //  0  起始符  STRING  固定为 ASCII 字符’view’
     uint8_t cmd;          //  2  命令单元  BYTE  命令单元定义见  表 A2  命令单元定义
     uint16_t data_len;    // 22  数据单元长度  WORD 数据单元长度是数据单元的总字节数，有效范围：0~65531
     void*   data;         // 24  数据单元    见数据单元格式和定义
     uint8_t BCC;          // 倒数第 1  校验码  BYTE 采用 BCC（异或校验）法，校验范围聪明星单元的第一个字节开始，同后一个字节异或，直到校验码前一字节为止，校验码占用一个字节
 };
 // login 包结构
 /*struct general_pack_view_login{
     char sn[18+1];        //  0  SN
     uint16_t data_len;    // 18  数据单元长度  WORD 数据单元长度是数据单元的总字节数，有效范围：0~65531
     char data[512];       // 20  数据单元    见数据单元格式和定义
 };
 // logout 包结构
 struct general_pack_view_logout{
     char sn[18+1];        //  0  SN
     uint16_t data_len;    // 18  数据单元长度  WORD 数据单元长度是数据单元的总字节数，有效范围：0~65531
     char data[512];       // 20  数据单元    见数据单元格式和定义
 };*/
 // OBD 包结构
 struct general_pack_view_obd{
     char sn[18+1];        //  0  SN
     uint16_t data_len;    // 18  数据单元长度  WORD 数据单元长度是数据单元的总字节数，有效范围：0~65531
     char data[512];       // 20  数据单元    见数据单元格式和定义
 };

#ifdef	__cplusplus
}
#endif

#endif // _SERVER_VIEW_H_
