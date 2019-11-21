#ifndef _MSG_PRINT_H_
#define _MSG_PRINT_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef	__cplusplus
extern "C" {
#endif

struct msg_print_obj;
struct msg_print_fops
{
    // 构造函数
    struct msg_print_obj* (*const constructed)(struct msg_print_obj* const _print_fops, void* const _obj_fops, const char _prefix[], char* const _stream, const size_t _n);
    int (*const init)(struct msg_print_obj* const _fops);
    int (*const print)(struct msg_print_obj* const _fops, const char *__format, ...);
    int (*const fflush)(struct msg_print_obj* const _fops);
    void (*const utc2filename)(struct msg_print_obj* const _fops, const uint32_t times, void* const buf, const size_t _size);
    void (*const date2filename)(struct msg_print_obj* const _fops, const uint32_t times, void* const buf, const size_t _size, const char* prefix);
    void (*const utc_format)(struct msg_print_obj* const _fops, const uint32_t times, uint8_t buf[], const size_t _size);
};

struct msg_print_obj
{
    const struct msg_print_fops* const fops; // 操作函数集合
    //char __stream[1024*30];  //
    char PathPrefix[64];
    char* const __stream;  //
    const size_t __n;
};

extern const struct msg_print_fops _msg_print_fops;
extern struct msg_print_obj _msg_obj;

#ifdef	__cplusplus
}
#endif

#endif // _MSG_PRINT_H_
