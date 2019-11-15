/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : obd_agree_yunjing.h
* Author             : Merafour
* Last Modified Date : 11/12/2019
* Description        : 《上海市车载排放诊断（OBD）系统在线接入技术指南（试行）发布稿.pdf》.
*                    :  云景客户修改其中部分指令
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#ifndef _OBD_AGREE_YUNJING_H
#define _OBD_AGREE_YUNJING_H

#include <stdint.h>
#include "agreement.h"
#include "obd_agree_shanghai.h"
#include "obd_agree_yunjing.h"
#include "obd_agree_fops.h"

#ifdef __cplusplus
 extern "C" {
#endif

 // 命令单元定义
enum cmd_unit_yunjing{
     CMD_LOGIN_YJ         = 0x01,       // 0x01  车辆登入  上行
     CMD_REPORT_REAL_YJ   = 0x02,       // 0x02  实时信息上报  上行
     CMD_REPORT_LATER_YJ  = 0x03,       // 0x03  补发信息上报  上行
     CMD_LOGOUT_YJ        = 0x04,       // 0x04  车辆登出  上行
     CMD_UTC_YJ           = 0x05,       // 0x05  终端校时  上行
     CMD_REV_YJ           = 0x06,       // 0x06~0x7F  上行数据系统预留  上行
     CMD_USERDEF_YJ       = 0x81,       // 0x81~0xFE  用户自定义
     CMD_YJ_NULL          = 0x00,       // error
     // 云景修改了内容重新定义命令号
     //CMD_LOGIN_YJ      = 0x01+0x85,  // 0x01  车辆登入  上行, 云景修改了内容重新定义命令号
     //CMD_LOGOUT_YJ     = 0x04+0x85,  // 0x04  车辆登出  上行, 云景修改了内容重新定义命令号
};

 struct yunjing_login{
     uint8_t UTC[6+1];       // 0  数据采集时间  BYTE[6]  时间定义见 A4.4
     uint16_t count;         // 6  登入流水号  WORD 车载终端每登入一次，登入流水号自动加 1，从 1开始循环累加，最大值为 65531，循环周期为天。
     uint8_t sn[18+1];       // 8  设备序列号由 6 位固定编码和 12 位厂家自定义序号(不重复)共 18 位组成。详情看附件设备序列号要求
 };
 struct yunjing_logout{
     uint8_t UTC[6+1];       // 0  数据采集时间  BYTE[6]  时间定义见 A4.4
     uint16_t count;         // 6  登入流水号  WORD 车载终端每登入一次，登入流水号自动加 1，从 1开始循环累加，最大值为 65531，循环周期为天。
     uint8_t sn[18+1];       // 8  设备序列号由 6 位固定编码和 12 位厂家自定义序号(不重复)共 18 位组成。详情看附件设备序列号要求
 };

 // 0x81~0xFE  用户自定义
 enum yunjing_userdef_cmd{
     USERDEF_YJ_QUERY_VIN     = 0x01,  // 请求 VIN 码
     USERDEF_YJ_ACK_VIN       = 0x02,  // 下发 VIN 码
     USERDEF_YJ_QUERY_TIME    = 0x03,  // 请求校时
     USERDEF_YJ_ACK_TIME      = 0x04,  // 下发校时
     USERDEF_YJ_DEV_FAULT     = 0x05,  // 设备故障
     USERDEF_YJ_DEV_OFFLINE   = 0x06,  // 设备离线
     // 下载
     USERDEF_YJ_QUERY_CFG     = 0x07,  // 查询配置文件更新
     USERDEF_YJ_ACK_CFG       = 0x08,  // 响应
     USERDEF_YJ_QUERY_FW      = 0x09,  // 查询固件更新
     USERDEF_YJ_ACK_FW        = 0x0A,  // 响应
     USERDEF_YJ_QUERY_FWB     = 0x0B,  // 查询固件更新,获取固件块信息
     USERDEF_YJ_ACK_FWB       = 0x0C,  // 响应
     USERDEF_YJ_DOWNLOAD      = 0x0D,  // 下载
     USERDEF_YJ_ACK_DOWNLOAD  = 0x0E,  // 响应
     USERDEF_YJ_PUSH          = 0x0F,  // 推送
     // 0x10-0xFF 预留
 };
 struct yunjing_userdef{
     uint16_t count;       // 信息流水号
     uint8_t type_msg;     // 信息类型标志
     char msg[1024+128];
 };
 struct userdef_yj_fault{
     uint8_t sum;
     uint16_t value[253];
 };
 // 查询
 struct userdef_yj_qure{
     char sn[18]; // SN
     uint32_t checksum;
 };
 struct userdef_yj_qure_ack{
     char key[32];           // 关键字,可以是文件名或其它便于服务器找到对应下载文件的信息,设备不失败其内容，下载文件时返回给服务器用于下载文件
     uint32_t checksum;      // 预下载文件的校验值,用于给设备判断下载是否出错
     uint32_t _total;        // 文件大小,0表示不需要更新
 };
 struct userdef_yj_qureb_ack{ // 块查询
     char key[32];            // 关键字,可以是文件名或其它便于服务器找到对应下载文件的信息,设备不失败其内容，下载文件时返回给服务器用于下载文件
     uint8_t map[32];         // 块信息表, 1bit表示一个块,0表示该块区有数据,1表示该块区全为1即无数据可跳过下载
     uint32_t checksum;       // 预下载文件的校验值,用于给设备判断下载是否出错
     uint32_t _total;         // 文件大小
     //uint16_t block;          // 块大小,map中1bit所代表的大小,必须是1K的整数倍
     uint8_t value ;          // 块填充值
     uint8_t block;           // 块大小,map中1bit所代表的大小,为 block*1024
 };
 struct userdef_yj_download{
     char key[32];           // 关键字,可以是文件名或其它便于服务器找到对应下载文件的信息,设备不失败其内容，下载文件时返回给服务器用于下载文件
     uint32_t _seek;         // 文件偏移
     uint32_t _total;         // 文件大小
     uint16_t block;         // 块大小,下载时每次下载的数据大小,设备根据自身资源设置,建议为 512*n(n为整数)
 };
 struct userdef_yj_ack_download{
     uint32_t _seek;         // 文件偏移
     uint32_t _total;         // 文件大小
     uint16_t block;         // 块大小,下载时每次下载的数据大小,设备根据自身资源设置,建议为 512*n(n为整数)
     char data[1024];        // 下载的数据大小
 };
 struct userdef_yj_push{     // 推送
     uint32_t checksum_cfg;  // 配置文件校验码
     uint32_t checksum_fw;   // 固件校验码
 };

 //extern int userdef_encode_yj(const struct yunjing_userdef *const _udef, void* const _buffer, const uint16_t _size);
 //extern int userdef_decode_yj(struct yunjing_userdef *const _udef, const void* const _data, const uint16_t _size);

#ifdef __cplusplus
}
#endif

#endif // _OBD_AGREE_YUNJING_H
