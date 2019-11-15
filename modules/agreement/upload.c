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
#include "upload.h"
#include "agreement.h"
#include <string.h>
#include <stdio.h>


static struct upload load;
struct upload* upload_init(const enum upload_cmd _cmd, const uint32_t pack_total, const uint32_t pack_index, const uint32_t checksum, const char Model[16], const void* const data, const uint16_t data_len)
{
    load.cmd = (uint8_t)_cmd;
    load.pack_total = pack_total;
    load.pack_index = pack_index;
    load.checksum   = checksum;
    load.down_len   = 0;
    memcpy(load.Model, Model, sizeof (load.Model));
    load.data_len = data_len;
    memcpy(load.data, data, data_len);
    return &load;
}
/**
 *  |  1  |   4   |   4   |   4   |   16  |  2   |  2  |  n   |  1  |
 *  | cmd | total | index | check | Model | down | len | data | CRC |
 */
int upload_encode(const struct upload *const _load, void* const _buffer, const uint16_t _size)
{
    uint16_t index;
    uint16_t i;
    uint8_t crc;
    uint8_t* const buffer = (uint8_t* const)_buffer;
    index=0;
    buffer[index++] = _load->cmd;
    index += bigw_32bit(&buffer[index], _load->pack_total);
    index += bigw_32bit(&buffer[index], _load->pack_index);
    index += bigw_32bit(&buffer[index], _load->checksum);
    memcpy(&buffer[index], _load->Model, sizeof (_load->Model)); index += sizeof (_load->Model);
    BUILD_BUG_ON(sizeof (_load->Model) != 16);
    index += bigw_16bit(&buffer[index], _load->down_len);
    index += bigw_16bit(&buffer[index], _load->data_len);
    if(_size<(index+_load->data_len+1)) return -1;
    memcpy(&buffer[index], _load->data, _load->data_len); index += _load->data_len;
    crc=0;
    for(i=0; i<index; i++)
    {
        crc = crc + buffer[i];
    }
    buffer[index++] = crc;
    return index;
}

int upload_decode(struct upload *_load, const void* const _data, const uint16_t _size)
{
    uint16_t index;
    uint16_t i;
    uint8_t crc;
    const uint8_t* const data = (const uint8_t* const)_data;
    index=0;
    _load->cmd = data[index++];
    _load->pack_total = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
    _load->pack_index = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
    _load->checksum   = merge_32bit(data[index], data[index+1], data[index+2], data[index+3]); index+= 4;
    memcpy(_load->Model, &data[index], sizeof (_load->Model)); index += sizeof (_load->Model);
    BUILD_BUG_ON(sizeof (_load->Model) != 16);
    _load->down_len = merge_16bit(data[index], data[index+1]); index+= 2;
    _load->data_len = merge_16bit(data[index], data[index+1]); index+= 2;
    if(_size<(index+_load->data_len+1)) return -1;
    memcpy(_load->data, &data[index], _load->data_len); index += _load->data_len;
    crc=0;
    for(i=0; i<index; i++)
    {
        crc = crc + data[i];
    }
    if(crc != data[index]) return -1;
    _load->CRC = data[index++];
    return index;
}

int upload_test()
{
    uint8_t buffer[sizeof(struct upload)];
    struct upload decode;
    int len;
    struct upload* _load = upload_init(UPLOAD_LOGIN, 0, 0, 0x12345678, "OBD1234567890ABCDEF", "Hello", 5);
    memset(buffer, 0, sizeof (buffer));
    memset(&decode, 0, sizeof (decode));
    len = upload_encode(_load, buffer, sizeof (buffer));
    printf("encode len : %d\n", len); fflush(stdout);
    len = upload_decode(&decode, buffer, len);
    if(len<0)
    {
        printf("decode error!\n"); fflush(stdout);
    }
    else
    {
        printf("decode cmd : 0x%02X\n", decode.cmd); fflush(stdout);
        printf("decode total : %d\n", decode.pack_total); fflush(stdout);
        printf("decode index : %d\n", decode.pack_index); fflush(stdout);
        printf("decode checksum : 0x%02X\n", decode.checksum); fflush(stdout);
        printf("decode Model : %s\n", decode.Model); fflush(stdout);
        printf("decode len : %d\n", decode.data_len); fflush(stdout);
        printf("decode data : %s\n", decode.data); fflush(stdout);
        printf("decode CRC : 0x%02X\n", decode.CRC); fflush(stdout);
    }
    return 0;
}
