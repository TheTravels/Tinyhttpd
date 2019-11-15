/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : config_load.h
* Author             : Merafour
* Last Modified Date : 11/15/2019
* Description        : 加载配置文件并解析.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/

#ifndef CONFIG_LOAD_H_
#define CONFIG_LOAD_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct config_load_obj;
typedef int (*config_load_func_t)(struct config_load_obj* const _load_obj);
struct config_load_fops{
    // 构造函数
    //struct config_load_obj* (*const constructed)(struct config_load_obj* const _load_obj, void* const _obj_buf, const char _cfg_path[], char* const _stream, const size_t _n);
    struct config_load_obj* (*const constructed)(struct config_load_obj* const _load_obj, void* const _obj_buf, const config_load_func_t _load_func, const char _cfg_path[], char* const _stream, const size_t _n, void* const _data);
    int (*const load)(struct config_load_obj* const _load_obj);  // 加载配置文件到内存
    int (*const get_field_value)(struct config_load_obj* const _load_obj, const char * const section, const char* const _key, const char* const dft, char* const _value);
    int (*const get_int)(struct config_load_obj* const _load_obj, const char * const section, const char* const _key, const int dft);
    double (*const get_double)(struct config_load_obj* const _load_obj, const char * const section, const char* const _key, const double dft);
    int (*const read_key_value)(const char * const section, const char* const _key, const char* const dft, char* const _value, char * const _path);
    int (*const read_int)(const char * const section, const char* const _key, const int dft, char * const _path);
    double (*const read_double)(const char * const section, const char* const _key, const double dft, char * const _path);
};

struct config_load_obj{
    //const struct config_load_fops* const fops;
    const struct config_load_fops fops;
    int (*const load)(struct config_load_obj* const _load_obj);
    const char _cfg_path[128];          // 配置文件路径
    char* const _stream;                // 内存缓存文件
    const size_t _n;                    // 缓存大小
    size_t _len;                        // 数据长度
    void* const data;                   // 数据
};

//extern const struct config_load_fops config_load_fops;
extern struct config_load_obj config_load_dft;

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_LOAD_H_ */
