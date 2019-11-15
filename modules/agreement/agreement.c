/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : agreement.c
* Author             : Merafour
* Last Modified Date : 11/12/2019
* Description        : OBD agreement.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#include "agreement.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include "upload.h"

#define  MCU_BUILD   0
#if MCU_BUILD
// 下载临时路径
const char* download_cfg_temp="/Upgrade.cfg";
const char* download_fw_temp="/Upgrade.bin";
const char* config_path="/OBDII.cfg";
const char* fw_path="/download.bin";
const char* download_cache_path="/Download.dat";
#else
// 下载临时路径
const char* download_cfg_temp="./upload/Upgrade.cfg";
const char* download_fw_temp="./upload/Upgrade.bin";
const char* config_path="./upload/OBDII.cfg";
const char* fw_path="./upload/download.bin";
const char* download_cache_path="./upload/Download.dat";
#endif
// 临时 VIN
const char* Temporary_vin = "VINTT23456789ABCDEF";

//// Verify that this architecture packs as expected.
//#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
///* 取对应字节 */
//#define BYTE0(data)    (data&0xFF)
//#define BYTE1(data)    ((data>>8)&0xFF)
//#define BYTE2(data)    ((data>>16)&0xFF)
//#define BYTE3(data)    ((data>>24)&0xFF)
/* 取对应4bit */
//#define HEXL4(data)    (data&0xF)
//#define HEXH4(data)    ((data>>4)&0xF)

uint8_t hexl4(const uint8_t data)
{
    uint8_t hex;
    hex = data&0xF;
    if(hex<=9) hex = hex + '0'; // ASCII
    else hex = 'A' + hex - 10;  // ASCII
    return hex;
}
uint8_t hexh4(const uint8_t data)
{
    uint8_t hex;
    hex = (data>>4)&0xF;
    if(hex<=9) hex = hex + '0'; // ASCII
    else hex = 'A' + hex - 10;  // ASCII
    return hex;
}
uint8_t hex2int(const uint8_t hex)
{
    uint8_t data=0;
    if(('0'<=hex) && (hex<='9')) data = hex - '0'; // uint8_t
    else if(('A'<=hex) && (hex<='F')) data = hex + 10 - 'A'; // uint8_t
    else if(('a'<=hex) && (hex<='f')) data = hex + 10 - 'a'; // uint8_t
    else data = 0xFF; // error
    return data;
}

/* 大端存储 */
uint32_t bigw_16bit(uint8_t buf[], const uint16_t data)
{
    buf[0] = BYTE1(data);
    buf[1] = BYTE0(data);
    return 2;
}
uint32_t bigw_32bit(uint8_t buf[], const uint32_t data)
{
    buf[0] = BYTE3(data);
    buf[1] = BYTE2(data);
    buf[2] = BYTE1(data);
    buf[3] = BYTE0(data);
    return 4;
}
uint16_t merge_16bit(const uint8_t byte0, const uint8_t byte1)
{
    uint16_t data=0;
    data = byte0;
    data = (data<<8) | byte1;
    return data;
}
uint32_t merge_32bit(const uint8_t byte0, const uint8_t byte1, const uint8_t byte2, const uint8_t byte3)
{
    uint32_t data=0;
    data = byte0;
    data = data<<8 | byte1;
    data = data<<8 | byte2;
    data = data<<8 | byte3;
    return data;
}

// 校验码  倒数第 3  String[2]  2 采用 BCC 异或校验法，由两个 ASCII 码（高位在前，低位在后）组成 8 位校验码。校验范围从接入层协议类型开始到数据单元的最后一个字节。
uint8_t BCC_check_code(const uint8_t data[], const uint32_t len)
{
    uint8_t bcc=0;
    uint32_t count=0;
    bcc = data[0];
    for(count=1; count<len; count++)
    {
        bcc = bcc ^ data[count];
    }
    return bcc;
}

// 数据转换
//int convert_data(const int _data, const int offset)
float FloatConvert(const int _data, const float offset, const float precision)
{
#if 0
    float data = _data+offset;
    data = data * precision;  // 转换精度
#else
    float data = _data * precision;
    data = data+offset;  // 转换精度
#endif
    return data;
}
int IntConvert(const int _data, const float offset, const float precision)
{
    float data = FloatConvert(_data, offset, precision);
    return (int)data;
}
double DIntConvert(const uint32_t _data, const double offset, const double precision)
{
    double data = _data;
    data = data * precision;
    data = data+offset;  // 转换精度
    return data;
}

#if 1
uint32_t crc_form_file(const char* filename, void* const buffer, const uint32_t bsize)
{
    char* const filebin = (char* const)buffer; // 1024B
    long _size=0;
    long _seek=0;
    long rsize=0;
    long _rsize=0;
    (void)_rsize;
    //size_t count=0;
    uint32_t checksum = 0x12345678;
    unsigned short sum = 0;
    FILE* fd = fopen(filename, "rb");
    if(NULL == fd)
    {
        ;//printf("file %s not exist! \n", filename);  fflush(stdout);
    }
    else
    {
        fseek(fd, 0, SEEK_END);
        _size = ftell(fd);
        fseek(fd, 0, SEEK_SET);
        //printf("%s@%d bsize:%d _size:%ld\n", __func__, __LINE__, bsize, _size); fflush(stdout);
        rsize=0;
        checksum = 0;
        sum = 0;
        for(_seek=0; _seek<_size; _seek+=rsize)
        {
            rsize = _size - _seek;
            if(rsize>bsize) rsize=bsize;
            // read
            memset(filebin, 0xFF, bsize);
            _rsize = fread(filebin, (size_t)rsize, 1, fd);
            //printf("%s@%d rsize:[%ld %ld] _size:%ld _seek:%ld\n", __func__, __LINE__, rsize, _rsize, _size, _seek); fflush(stdout);
            sum = fast_crc16(sum&0xFFFF, (unsigned char *)filebin, rsize);
        }
        fclose(fd);
        checksum = sum;
        //printf("%s@%d fread _size:%d count:%d\n", __func__, __LINE__, _size, count); fflush(stdout);
        //if(1!=count) return NULL;
        //pr_debug("%s@%d fread _size:%d\n", __func__, __LINE__, _size);
    }
    return checksum;
}
#else
#include <stdio.h>
#include<fcntl.h>
uint32_t crc_form_file(const char* filename, void* const buffer, const uint32_t bsize)
{
    char* const filebin = (char*)buffer; // 1024B
    long _size=0;
    long _seek=0;
    long rsize=0;
    long _rsize=0;
    (void) _rsize;
    //size_t count=0;
    uint32_t checksum = 0x12345678;
    unsigned short sum = 0;
    int fd = open(filename, O_RDONLY|O_BINARY, 0);
    if(0 > fd)
    {
        printf("file %s not exist!..1 \n", filename);  fflush(stdout);
    }
    else
    {
        _size = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        printf("%s@%d bsize:%d _size:%ld\n", __func__, __LINE__, bsize, _size); fflush(stdout);
        rsize=0;
        checksum = 0;
        sum = 0;
        for(_seek=0; _seek<_size; _seek+=rsize)
        {
            rsize = _size - _seek;
            if(rsize>bsize) rsize=bsize;
            // read
            memset(filebin, 0xFF, bsize);
            lseek(fd, _seek, SEEK_SET);
            _rsize = read(fd, filebin, (size_t)rsize);
            //printf("%s@%d rsize:[%ld %ld] _size:%ld _seek:%ld\n", __func__, __LINE__, rsize, _rsize, _size, _seek); fflush(stdout);
            sum = fast_crc16(sum&0xFFFF, (unsigned char *)filebin, rsize);
        }
        close(fd);
        checksum = sum;
        //pr_debug("%s@%d fread _size:%d count:%d\n", __func__, __LINE__, _size, count); fflush(stdout);
        //if(1!=count) return NULL;
        //pr_debug("%s@%d fread _size:%d\n", __func__, __LINE__, _size);
    }
    return checksum;
}
#endif
#include "DateTime.h"
// GMT+8 时间，时间定义符合GB/T32960.3-2016 第 6.4 条的要求
void UTC2GMT8(const uint32_t times, uint8_t buf[], const size_t _size)
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
}


