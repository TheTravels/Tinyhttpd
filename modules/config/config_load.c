/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : config_load.c
* Author             : Merafour
* Last Modified Date : 11/15/2019
* Description        : 加载配置文件并解析.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include "config_load.h"

//#include"magicNum.h"
#ifndef COMMONSTRUCT_H_
#define COMMONSTRUCT_H_

//#include</usr/include/linux/types.h>

enum _epollEvent{WEVENT = 1,REVENT};
typedef enum _epollEvent enumEpollEvent;

struct _msghead{
    size_t msgtype;
    size_t msgbodysize;
};
typedef struct _msghead msghead;

struct _msgdata{
    size_t datasize;
    char* databuf;
};
typedef struct _msgdata msgdata;

#endif /* COMMONSTRUCT_H_ */
#ifndef MAGICNUM_H_
#define MAGICNUM_H_
//魔法数一定不要直接使用

#define CSUCCESS 0
#define CFAILED -1

enum keylen{ KEYVALLEN = 100};
#define CFGPATH "./cfg.data"

#endif /* MAGICNUM_H_ */


/*   删除左边的空格   */
static char * l_trim(char * szOutput, const char *szInput)
{
	assert(szInput != NULL);
	assert(szOutput != NULL);
	assert(szOutput != szInput);
	for(; *szInput != '\0' && isspace(*szInput); ++szInput){
		;
	}
	return strcpy(szOutput, szInput);
}

/*   删除两边的空格   */
static char * a_trim(char * szOutput, const char * szInput)
		{
	char *p = NULL;
	assert(szInput != NULL);
	assert(szOutput != NULL);
	l_trim(szOutput, szInput);
	for   (p = szOutput + strlen(szOutput) - 1;p >= szOutput && isspace(*p); --p){
		;
	}
	*(++p) = '\0';
	return szOutput;
		}

static int __read_key_value(const char * const AppName, const char* const KeyName, char* const KeyVal, const char* const _path)
{
	char appname[32],keyname[32];
	char *buf,*c;
	char buf_i[KEYVALLEN], buf_o[KEYVALLEN];
	FILE *fp;
	int found=0; /* 1 AppName 2 KeyName */
    if( (fp=fopen( _path, "r" ))==NULL ){
        printf( "openfile [%s] error [%s]\n", _path, strerror(errno) );
		return(-1);
	}
	fseek( fp, 0, SEEK_SET );
	memset( appname, 0, sizeof(appname) );
	sprintf( appname,"[%s]", AppName );

	while( !feof(fp) && fgets( buf_i, KEYVALLEN, fp )!=NULL ){
		l_trim(buf_o, buf_i);
		if( strlen(buf_o) <= 0 )
			continue;
		buf = NULL;
		buf = buf_o;

		if( found == 0 ){
			if( buf[0] != '[' ) {
				continue;
			} else if ( strncmp(buf,appname,strlen(appname))==0 ){
				found = 1;
				continue;
			}

		} else if( found == 1 ){
			if( buf[0] == '#' ){
				continue;
			} else if ( buf[0] == '[' ) {
				break;
			} else {
				if( (c = (char*)strchr(buf, '=')) == NULL )
					continue;
				memset( keyname, 0, sizeof(keyname) );

				sscanf( buf, "%[^=|^ |^\t]", keyname );
				if( strcmp(keyname, KeyName) == 0 ){
					sscanf( ++c, "%[^\n]", KeyVal );
                    //char *KeyVal_o = (char *)malloc(strlen(KeyVal) + 1);
                    int _size = strlen(KeyVal) + 1;
                    char *KeyVal_o = (char *)malloc(_size);
					if(KeyVal_o != NULL){
                        //memset(KeyVal_o, 0, sizeof(KeyVal_o));
                        memset(KeyVal_o, 0, _size);
						a_trim(KeyVal_o, KeyVal);
						if(KeyVal_o && strlen(KeyVal_o) > 0)
							strcpy(KeyVal, KeyVal_o);
						free(KeyVal_o);
						KeyVal_o = NULL;
					}
					found = 2;
					break;
				} else {
					continue;
				}
			}
		}
	}
	fclose( fp );
	if( found == 2 )
		return(0);
	else
		return(-1);
}

struct file_serach {
  const char *const _text;  // 文件内容
  const int _stext;         // 文件大小
  int pos;                  // 当前文件指针位置
  //int _cnt;
  //int _flag;
};

char* serach_gets(char* const str, const int n, struct file_serach* const stream)
{
    int _end;
    int count=0;
    const char* const _text = stream->_text;
    char _byte=0;
    _end = stream->pos + n;
    memset(str, 0, (size_t)n);
    for(count=0; count<n; count++)
    {
        if(stream->pos>=stream->_stext) break;
        if(stream->pos>=_end) break;
        _byte = _text[stream->pos++];
        if('\n'==_byte) break;
        if('\0'==_byte) break;
        str[count] = _byte;
    }
    str[count] = '\0';
    if(0==count) return  NULL;
    return  str;
}

static int __get_field_value(const char * const AppName, const char* const KeyName, char* const KeyVal, const char _text[], const int _stext)
{
    char appname[32],keyname[32];
    char *buf,*c;
    char buf_i[KEYVALLEN], buf_o[KEYVALLEN];
    int found=0; /* 1 AppName 2 KeyName */
//    size_t _serach;
//    size_t _serach_end;
//    char _byte=0;
//    uint16_t count=0;
    struct file_serach fp = {_text, _stext, 0};
    memset( appname, 0, sizeof(appname) );
    sprintf( appname,"[%s]", AppName );

    //_serach = 0;
    //while(_serach<_stext)
    while( (fp.pos<fp._stext) && serach_gets( buf_i, KEYVALLEN, &fp )!=NULL )
    {
        // 读取一行
//        _serach_end = _serach + KEYVALLEN;
//        for(count=0; count<KEYVALLEN; count++)
//        {
//            if(_serach>=_stext) break;
//            if(_serach>=_serach_end) break;
//            _byte = _text[_serach++];
//            if('\n'==_byte) break;
//            buf_i[count] = _byte;
//        }
//        if(0==count) break;
        l_trim(buf_o, buf_i);
        if( strlen(buf_o) <= 0 )
            continue;
        buf = NULL;
        buf = buf_o;

        if( found == 0 )
        {
            if( buf[0] != '[' )
            {
                continue;
            }
            else if ( strncmp(buf,appname,strlen(appname))==0 )
            {
                found = 1;
                continue;
            }
        }
        else if( found == 1 )
        {
            if('#'==buf[0]) // if( buf[0] == '#' )
            {
                continue;
            }
            else if ( buf[0] == '[' )
            {
                break;
            }
            else
            {
                if( (c = (char*)strchr(buf, '=')) == NULL )
                    continue;
                memset( keyname, 0, sizeof(keyname) );

                sscanf( buf, "%[^=|^ |^\t]", keyname );
                if( strcmp(keyname, KeyName) == 0 )
                {
                    sscanf( ++c, "%[^\n]", KeyVal );
                    //char *KeyVal_o = (char *)malloc(strlen(KeyVal) + 1);
                    int _size = strlen(KeyVal) + 1;
                    char *KeyVal_o = (char *)malloc(_size);
                    if(KeyVal_o != NULL)
                    {
                        //memset(KeyVal_o, 0, sizeof(KeyVal_o));
                        memset(KeyVal_o, 0, _size);
                        a_trim(KeyVal_o, KeyVal);
                        if(KeyVal_o && strlen(KeyVal_o) > 0)
                            strcpy(KeyVal, KeyVal_o);
                        free(KeyVal_o);
                        KeyVal_o = NULL;
                    }
                    found = 2;
                    break;
                }
                else
                {
                    continue;
                }
            }
        }
    }
    if( found == 2 ) return(0);
    else return(-1);
}
// 构造函数
static struct config_load_obj* __constructed(struct config_load_obj* const _load_obj, void* const _obj_buf, const config_load_func_t _load_func, const char _cfg_path[], char* const _stream, const size_t _n, void* const _data)
{
    struct config_load_obj _fops = {
        .fops = _load_obj->fops,
        .load = _load_func,
        ._cfg_path = "./cfg.data",
        ._stream = _stream,
        ._n = _n,
        .data = _data,
    };
    char* const _path = (char*)_fops._cfg_path;
    struct config_load_obj* const _obj = (struct config_load_obj*)_obj_buf;
    //printf("[%s-%d] _n:%d\n", __func__, __LINE__, _n);  fflush(stdout);
    memset(_path, 0, sizeof(_fops._cfg_path));
    memcpy(_path, _cfg_path, strlen(_cfg_path));
    memcpy(_obj_buf, &_fops, sizeof(_fops));
    //printf("[%s-%d] _obj->_n:%d _fops->_n:%d\n", __func__, __LINE__, _obj->_n, _fops._n);  fflush(stdout);
    return _obj;
}
static struct config_load_obj* config_load_constructed(struct config_load_obj* const _load_obj, void* const _obj_buf, const config_load_func_t _load_func, const char _cfg_path[], char* const _stream, const size_t _n, void* const _data)
{
    config_load_func_t _func = _load_func;
    if(NULL == _func) _func = _load_obj->load;
    return __constructed(_load_obj, _obj_buf, _func, _cfg_path, _stream, _n, _data);
}

static int __config_load(struct config_load_obj* const _load_obj)
{
    FILE* fd = NULL;
    long _size=0;
    //long _rsize=0;
    const long _n=(long)_load_obj->_n;
    const char *const path = _load_obj->_cfg_path;

    fd = fopen(path, "r");  //
    if(NULL==fd)
    {
        printf("[%s-%d]fopen %s fail! error [%s]\n", __func__, __LINE__, path, strerror(errno)); fflush(stdout);
        return -1;
    }
    fseek(fd, 0, SEEK_END);
    _size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    //printf("_size %s :%ld _n:%ld\n", path, _size, _n);  fflush(stdout);
    if(_size>_n) _size=_n;
    fread(_load_obj->_stream, (size_t)_size, 1, fd);
    fclose(fd);
    //printf("_size %s :%ld \n%s\n", path, _size, _load_obj->_stream);  fflush(stdout);

    //pr_debug("data:\n%s\n", json_buf); fflush(stdout);

    _load_obj->_len = (size_t)_size;
    return _size;
}
static int config_load(struct config_load_obj* const _load_obj)  // 加载配置文件到内存
{
    //printf("[%s-%d] _n:%d\n", __func__, __LINE__, _load_obj->_n);  fflush(stdout);
    if(0==_load_obj->_n) return 0;
    memset(_load_obj->_stream, 0, _load_obj->_n);
    if(__config_load(_load_obj)>0)
    {
        return 0;
    }
    return -1;
}

static int get_field_value(struct config_load_obj* const _load_obj, const char * const section, const char* const _key, const char* const dft, char* const _value)
{
    int dft_size = strlen(dft);
    int ret = __get_field_value(section, _key, _value, _load_obj->_stream, _load_obj->_len);
    if(0!=ret)
    {
        memcpy(_value, dft, dft_size);
        _value[dft_size] = '\0';
    }
    return 0;
}
static int get_int(struct config_load_obj* const _load_obj, const char * const section, const char* const _key, const int dft)
{
    char _value[512];
    char _dft[32];
    int value=dft;
    memset(_value, 0, sizeof(_value));
    memset(_dft, 0, sizeof(_dft));
    snprintf(_dft, sizeof(_dft)-1, "%8d", dft);
    int ret = _load_obj->fops.get_field_value(_load_obj, section, _key, _dft, _value);
    if(0==ret)
    {
        // char to int
        value = atoi(_value);
    }
    return value;
}
static double get_double(struct config_load_obj* const _load_obj, const char * const section, const char* const _key, const double dft)
{
    char _value[512];
    char _dft[32];
    double value=dft;
    memset(_value, 0, sizeof(_value));
    memset(_dft, 0, sizeof(_dft));
    snprintf(_dft, sizeof(_dft)-1, "%8f", dft);
    int ret = _load_obj->fops.get_field_value(_load_obj, section, _key, _dft, _value);
    if(0==ret)
    {
        // char to int
        value = atof(_value);
    }
    return value;
}
static int read_key_value(const char * const section, const char* const _key, const char* const dft, char* const _value, char * const _path)
{
    int dft_size = strlen(dft);
    int ret = __read_key_value(section, _key, _value, _path);
    if(0!=ret)
    {
        memcpy(_value, dft, dft_size);
        _value[dft_size] = '\0';
    }
    return 0;
}
static int read_int(const char * const section, const char* const _key, const int dft, char * const _path)
{
    char _value[512];
    char _dft[32];
    int value=dft;
    memset(_value, 0, sizeof(_value));
    memset(_dft, 0, sizeof(_dft));
    snprintf(_dft, sizeof(_dft)-1, "%8d", dft);
    int ret = read_key_value(section, _key, _dft, _value, _path);
    if(0==ret)
    {
        // char to int
        value = atoi(_value);
    }
    return value;
}
static double read_double(const char * const section, const char* const _key, const double dft, char * const _path)
{
    char _value[512];
    char _dft[32];
    double value=dft;
    memset(_value, 0, sizeof(_value));
    memset(_dft, 0, sizeof(_dft));
    snprintf(_dft, sizeof(_dft)-1, "%8f", dft);
    int ret = read_key_value(section, _key, _dft, _value, _path);
    if(0==ret)
    {
        // char to int
        value = atof(_value);
    }
    return value;
}

static int obj_load(struct config_load_obj* const _load_obj)
{
    int ret=-1;
    printf("[%s-%d]\n", __func__, __LINE__);  fflush(stdout);
    ret = _load_obj->fops.load(_load_obj);
    return ret;
}
/*const struct config_load_fops config_load_fops = {
    .constructed = config_load_constructed,
    .load = config_load,
    .get_field_value = get_field_value,
    .get_int = get_int,
    .get_double = get_double,
    .read_key_value = read_key_value,
    .read_int = read_int,
    .read_double = read_double,
};*/
struct config_load_obj config_load_dft = {
    //.fops = &config_load_fops,
    .fops = {
        .constructed = config_load_constructed,
        .load = config_load,
        .get_field_value = get_field_value,
        .get_int = get_int,
        .get_double = get_double,
        .read_key_value = read_key_value,
        .read_int = read_int,
        .read_double = read_double,
    },
    .load = obj_load,
    ._cfg_path = "./cfg.data",
    ._stream = NULL,
    ._n = 0,
    ._len = 0,
    .data = NULL,
};
