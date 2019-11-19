/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : obd_agree_shanghai.h
* Author             : Merafour
* Last Modified Date : 11/12/2019
* Description        : 《上海市车载排放诊断（OBD）系统在线接入技术指南（试行）发布稿.pdf》.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#ifndef _OBD_AGREE_SHANGHAI_H_
#define _OBD_AGREE_SHANGHAI_H_

//#include "obd_agree_fops.h"

#include <stdint.h>
#include "agreement.h"

#ifdef	__cplusplus
extern "C" {
#endif

 // 命令单元定义
 enum cmd_unit_shanghai{
     CMD_LOGIN         = 0x01,       // 0x01  车辆登入  上行
     CMD_REPORT_REAL   = 0x02,       // 0x02  实时信息上报  上行
     CMD_REPORT_LATER  = 0x03,       // 0x03  补发信息上报  上行
     CMD_LOGOUT        = 0x04,       // 0x04  车辆登出  上行
     CMD_UTC           = 0x05,       // 0x05  终端校时  上行
     CMD_REV           = 0x06,       // 0x06~0x7F  上行数据系统预留  上行
     CMD_USERDEF       = 0x81,       // 0x81~0xFE  用户自定义
     CMD_NULL          = 0x00,       // error
     // 云景修改了内容重新定义命令号
     //CMD_LOGIN_YJ      = 0x01+0x85,  // 0x01  车辆登入  上行
     //CMD_LOGOUT_YJ     = 0x04+0x85,  // 0x04  车辆登出  上行
 };

 // 通用包结构
 struct general_pack_shanghai{
     uint8_t start[2];     //  0  起始符  STRING  固定为 ASCII 字符’##’，用“0x23，0x23”表示
     uint8_t cmd;          //  2  命令单元  BYTE  命令单元定义见  表 A2  命令单元定义
     uint8_t VIN[17+1];    //  3  车辆识别号  STRING 车辆识别码是识别的唯一标识，由 17 位字码组成，字码应符合 GB16735 中 4.5 的规定
     uint8_t soft_version; // 20  终端软件版本号  BYTE  终端软件版本号有效值范围 0~255
     uint8_t ssl;          // 21  数据加密方式  BYTE 0x01：数据不加密；0x02：数据经过 RSA 算法; 加密；0x03：数据经过国密 SM2 算法加密；“0xFE”标识异常，“0xFF”表示无效，其他预留
     uint16_t data_len;    // 22  数据单元长度  WORD 数据单元长度是数据单元的总字节数，有效范围：0~65531
     void*   data;         // 24  数据单元    见数据单元格式和定义
     uint8_t BCC;          // 倒数第 1  校验码  BYTE 采用 BCC（异或校验）法，校验范围聪明星单元的第一个字节开始，同后一个字节异或，直到校验码前一字节为止，校验码占用一个字节
 };

 struct shanghai_login{
     uint8_t UTC[6+1];       // 0  数据采集时间  BYTE[6]  时间定义见 A4.4
     uint16_t count;         // 6  登入流水号  WORD 车载终端每登入一次，登入流水号自动加 1，从 1开始循环累加，最大值为 65531，循环周期为天。
     uint8_t ICCID[20+1];    // 10  SIM 卡号  STRING SIM 卡 ICCID 号（ICCID 应为终端从 SIM 卡获取的值，不应人为填写或修改）。
 };
 // A4.5.3  车辆登出信息
 struct shanghai_logout{
     uint8_t UTC[6+1];       // 0  数据采集时间  BYTE[6]  时间定义见 A4.4
     uint16_t count;         // 6  登入流水号  WORD 车载终端每登入一次，登入流水号自动加 1，从 1开始循环累加，最大值为 65531，循环周期为天。
     // ;
 };
 enum shanghai_msg_type{
     MSG_OBD        = 0x01,   // 0x01  OBD 信息
     MSG_STREAM     = 0x02,   // 0x02  数据流信息
     // 0x03-0x7F  预留
     MSG_STREAM_ATT = 0x80,   // 0x80  补充数据流
     // 0x81~0xFE  用户自定义
     MSG_SMOKE      = 0x81,   // 0x81  包含烟雾的数据流信息(自定义)
 };
// // 单向链表，用于存放信息
// struct report_head{
//     struct report_head *next;     // 下一条信息
//     uint32_t data;                // 可携带四字节数据
//     uint8_t type_msg;             // 消息类型
//     uint8_t rev[3];
// };
 struct shanghai_report_real{
     uint8_t UTC[6+1];              // 0  数据采集时间  BYTE[6]  时间定义见 A4.4
     uint16_t count;              // 6  登入流水号  WORD 车载终端每登入一次，登入流水号自动加 1，从 1开始循环累加，最大值为 65531，循环周期为天。
     struct report_head* msg;     // 实时信息,考虑到会包含多条信息故用链表存放信息
 };
 // 表 A.6 OBD 信息数据格式和定义
 struct shanghai_data_obd{
     struct report_head head;
     uint8_t protocol;       // OBD 诊断协议  1  BYTE 有效范围 0~2，“0”代表 IOS15765，“1”代表IOS27145，“2”代表 SAEJ1939，“0xFE”表示无效。
     uint8_t MIL;            // MIL 状态  1  BYTE 有效范围 0~1，“0”代表未点亮，“1”代表点亮。“0xFE”表示无效。
     uint16_t status;        // 诊断支持状态  2  WORD
     uint16_t ready;         // 诊断就绪状态  2  WORD
     uint8_t VIN[17+1];        // 车辆识别码（VIN） 17  STRING
     uint8_t SVIN[18+1];       // 软件标定识别号 18  STRING 软件标定识别号由生产企业自定义，字母或数字组成，不足后面补字符“0”。
     uint8_t CVN[18+1];        // 标定验证码（CVN） 18  STRING 标定验证码由生产企业自定义，字母或数字组成，不足后面补字符“0”。
     //uint8_t IUPR[36+1];       // IUPR 值  36  DSTRING  定义参考 SAE J 1979-DA 表 G11.
     uint16_t IUPR[18];       // IUPR 值  36  DSTRING  定义参考 SAE J 1979-DA 表 G11.
     uint8_t fault_total;    // 故障码总数  1  BYTE  有效值范围：0~253，“0xFE”表示无效。
     struct report_head fault_list;  // 故障码信息列表 ∑每个故障码信息长度 N*BYTE（4） 每个故障码为四字节，可按故障实际顺序进行排序。
 };
 // 表 A.7 发动机数据流信息数据格式和定义
 struct shanghai_data_stream{
     struct report_head head; // 下一项数据指针
     uint16_t speed;          // 0  车速  WORD  km/h 数据长度：2btyes 精度：1/256km/h/ bit 偏移量：0 数据范围：0~250.996km/h “0xFF,0xFF”表示无效
     uint8_t  kPa;            // 2  大气压力  BYTE  kPa 数据长度：1btyes 精度：0.5/bit 偏移量：0 数据范围：0~125kPa “0xFF”表示无效
     uint8_t   Nm;            // 3  发动机净输出扭矩（实际扭矩百分比）  BYTE  %  数据长度：1btyes精度：1%/bit 偏移量：-125 数据范围：-125~125% “0xFF”表示无效
     uint8_t  Nmf;            // 4 摩擦扭矩（摩擦扭矩百分比） BYTE  % 数据长度：1btyes 精度：1%/bit 偏移量：-125 数据范围：-1250~125% “0xFF”表示无效
     uint16_t  rpm;            // 5  发动机转速  WORD  rpm 数据长度：2btyes 精度：0.125rpm/bit 偏移量：0 数据范围：0~8031.875rpm “0xFF,0xFF”表示无效
     uint16_t  Lh;            // 7  发动机燃料流量  WORD  L/h 数据长度：2btyes 精度：0.05L/h 偏移量：0 数据范围：0~3212.75L/h “0xFF,0xFF”表示无效
     uint16_t ppm_up;         // 9 SCR 上游 NOx 传感器输出值（后处理上游氮氧浓度） WORD  ppm 数据长度：2btyes 精度：0.05ppm/bit 偏移量：-200 数据范围：-200~3212.75ppm “0xFF,0xFF”表示无效
     uint16_t ppm_down;       // 11 SCR 下游 NOx 传感器输出值（后处理下游氮氧浓度） WORD  ppm 数据长度：2btyes 精度：0.05ppm/bit 偏移量：-200 数据范围：-200~3212.75ppm  “0xFF,0xFF”表示无效
     uint8_t urea_level;      // 13 反应剂余量 （尿素箱液位） BYTE  % 数据长度：1btyes 精度：0.4%/bit 偏移量：0 数据范围：0~100% “0xFF”表示无效
     uint16_t kgh;            // 14  进气量  WORD  kg/h 数据长度：2btyes 精度：0.05kg/h per bit 偏移量：0 数据范围：0~3212.75ppm “0xFF,0xFF”表示无效
     uint16_t SCR_in;         // 16 SCR 入口温度（后处理上游排气温度） WORD  ℃ 数据长度：2btyes 精度：0.03125 ℃ per bit 偏移量：-273 数据范围：-273~1734.96875℃ “0xFF,0xFF”表示无效
     uint16_t SCR_out;        // 18 SCR 出口温度（后处理下游排气温度） WORD  ℃ 数据长度：2btyes 精度：0.03125 ℃ per bit 偏移量：-273 数据范围：-273~1734.96875℃ “0xFF,0xFF”表示无效
     uint16_t DPF;            // 20 DPF 压差（或 DPF排气背压） WORD  kPa 数据长度：2btyes 精度：0.1 kPa per bit 偏移量：0 数据范围：0~6425.5 kPa “0xFF,0xFF”表示无效
     uint8_t coolant_temp;    // 22  发动机冷却液温度  BYTE  ℃ 数据长度：1btyes 精度：1 ℃/bit 偏移量：-40 数据范围：-40~210℃ “0xFF”表示无效
     uint8_t tank_level;      // 23  油箱液位  BYTE  % 数据长度：1btyes 精度：0.4% /bit 偏移量：0 数据范围：0~100% “0xFF”表示无效
     uint8_t gps_status;      // 24  定位状态  BYTE    数据长度：1btyes
     uint32_t longitude;      // 25  经度  DWORD   数据长度：4btyes 精度：0.000001° per bit 偏移量：0 数据范围：0~180.000000° “0xFF,0xFF,0xFF,0xFF”表示无效
     uint32_t latitude;       // 29  纬度  DWORD   数据长度：4btyes 精度：0.000001 度  per bit 偏移量：0 数据范围：0~180.000000° “0xFF,0xFF,0xFF,0xFF”表示无效
     uint32_t mileages_total; // 33 累计里程 （总行驶里程） DWORD  km 数据长度：4btyes 精度：0.1km per bit 偏移量：0 “0xFF,0xFF,0xFF,0xFF”表示无效

 };
 // 表 A.8 补充数据流数据格式和定义
 struct shanghai_data_att{
     struct report_head head;
     uint8_t Nm_mode;           // 0  发动机扭矩模式  BYTE    0：超速失效 1：转速控制 2：扭矩控制 3：转速/扭矩控制 9：正常
     uint8_t  accelerator;      // 1  油门踏板  BYTE  % 数据长度：1btyes 精度：0.4%/bit 偏移量：0 数据范围：0~100% “0xFF”表示无效
     uint32_t oil_consume;      // 2 累计油耗 （总油耗） DWORD  L 数据长度：4btyes 精度：0.5L per bit 偏移量：0 数据范围：0~2 105 540 607.5L “0xFF,0xFF,0xFF,0xFF”表示无效
     uint8_t urea_tank_temp;    // 6  尿素箱温度  BYTE  ℃ 数据长度：1btyes 精度：1 ℃/bit 偏移量：-40 数据范围：-40~210℃ “0xFF”表示无效
     uint32_t mlh_urea_actual;  // 7  实际尿素喷射量  DWORD  ml/h 数据长度：4btyes 精度：0.01 ml/h per bit 偏移量：0 数据范围：0 “0xFF,0xFF,0xFF,0xFF”表示无效
     uint32_t mlh_urea_total;   // 11 累计尿素消耗 （总尿素消耗） DWORD  g 数据长度：4btyes 精度：1 g per bit 偏移量：0 数据范围：0  “0xFF,0xFF,0xFF,0xFF”表示无效
     uint16_t exit_gas_temp;    // 15  DPF 排气温度  WORD  ℃ 数据长度：2btyes 精度：0.03125 ℃ per bit 偏移量：-273 数据范围：-273~1734.96875℃ “0xFF,0xFF”表示无效
     // ;
 };
 // 0x81~0xFE  用户自定义
 struct shanghai_userdef{
     const void* data;
     uint16_t _dsize;
 };

// 通用包结构
union general_data_unit_shanghai{
    struct shanghai_login login;
    struct shanghai_logout logout;
    struct shanghai_report_real report;
};

#ifdef	__cplusplus
}
#endif

#endif // _OBD_AGREE_SHANGHAI_H_
