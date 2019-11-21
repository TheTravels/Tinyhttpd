#include "cJSON.h"
#include "mem_malloc.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<stdarg.h>
#include "cfg.h"
#include "product.h"

static char json_buf[1024 * 50];
const char local_cfg_path[] = "./sn.cfg";
static const char cfg_log_path[] = "cfg.log";
static const uint32_t tea_key[4] = { 0x37452908, 0x37DE2CA8, 0x3BCD5908, 0x375FFEA8 };
static char msg_buf[1024 * 100];
static const uint32_t local_cfg_tea_iteration = 32;
#if 0
static int msgn_print(char* __stream, const size_t __n, const char* __format, ...)
{
	char* text = NULL;
	size_t _size = 0;
	//va_list ap;
	va_list args;
	_size = strlen(__stream);
	if (_size >= __n) return -1;
	text = &__stream[_size];
	/*va_start(ap, __format);
	//vprintf(__format, ap);
	//snprintf(text, sizeof (text), __format, ap);
	//vsprintf(text, __format, ap);
	vsnprintf(text, __n - _size, __format, ap);
	va_end(ap);*/
	//va_list args;
	va_start(args, __format);
	snprintf(text, __n - _size, __format, args);
	va_end(args);
	return 0;
}
#endif
static int msg_print(const char* fmt, ...)
{
	static char log_buf[1024] = { 0 };
	size_t _size = 0;
	va_list args;
    //size_t length;
	_size = strlen(msg_buf);
	memset(log_buf, 0, sizeof(log_buf));
	va_start(args, fmt);
    /*length = */vsnprintf(log_buf, sizeof(log_buf) - 1, fmt, args);
	//length = snprintf(&msg_buf[_size], length - 1, fmt, args);
	//vsprintf(log_buf, fmt, args);
	va_end(args);
	memcpy(&msg_buf[_size], log_buf, strlen(log_buf));
	//write_to_file(cfg_log_path, log_buf, strlen(log_buf));
	return 0;
}

void print_hex(const char _data[], const uint32_t _dsize)
{
	uint32_t _size = 0;
	msg_print("HEX:");
	for (_size = 0; _size < _dsize; _size++) msg_print("%02X ", _data[_size]&0xFF);
	msg_print("\n");
}
void print_hex_buf(char _buf[], const int _bsize, const char _data[], const uint32_t _dsize)
{
	uint32_t _size = 0;
    long len = 0;
	//char data;
	for (_size = 0; _size < _dsize; _size++)
	{
		//data = _data[_size] & 0xFF;
		len = strlen(_buf);
		sprintf(&_buf[len], "%02X ", _data[_size] & 0xFF);
	}
}

int write_to_file(const char path[], const char _data[], const uint32_t _dsize)
{
	FILE* fd = NULL;
	fd = fopen(path, "wb+");  // w+ : 可读可写, 可以不存在, 必会擦掉原有内容从头写
	if (NULL == fd)
	{
		printf("fopen fail!\n"); fflush(stdout);
		//mem_free(out);
		return -3;
	}
	fwrite(_data, _dsize, 1, fd);
	//fwrite("\n", 1, 1, fd);
	fflush(fd);
	fclose(fd);
	msg_print("[%s--] write size[%d]\n", __func__, _dsize);
	return 0;
}

int read_from_file(const char path[], char _data[], const uint32_t _dsize)
{
	FILE* fd = NULL;
	int _size;
	fd = fopen(path, "rb");
	if (NULL == fd)
	{
		return -1;
	}
	fseek(fd, 0, SEEK_END);
	_size = ftell(fd);
	msg_print("[%s--] ftell size[%d]\n", __func__, _size);
	if (_size > _dsize)
	{
		fclose(fd);
		return -2;
	}
	fseek(fd, 0, SEEK_SET);
	//printf("_size %s :%ld \n", path, _size);  
	memset(_data, 0, _dsize);
	fread(_data, _size, 1, fd);
	fclose(fd);
	msg_print("[%s--] fread size[%d]\n", __func__, _size);
	return _size;
}

void msg_fflush(void)
{
	write_to_file(cfg_log_path, msg_buf, strlen(msg_buf));
}
#if 0
static const char path[] = "OBD.cfg";

/**
{
    "cJSON Version":	"1.7.12",
    "Host":	"39.108.72.130",
    "Port":	9910,
    "CarType":	"FuelType ",
    "GPS_Time":	10,
    "Car_Protocol":	0,
    "CAN_bps":	500000
}
 */
int create_cfg(void)
{
    cJSON *_root = NULL;
    //cJSON *item_json = NULL;
    char *out = NULL;
    FILE* fd = NULL;

    _root = cJSON_CreateObject();
    cJSON_AddStringToObject(_root, "cJSON Version", cJSON_Version());
    cJSON_AddStringToObject(_root, "Host", "39.108.72.130");
    cJSON_AddNumberToObject(_root, "Port", 9910);
    cJSON_AddStringToObject(_root, "CarType", "FuelType ");   // 汽油车
    //cJSON_AddStringToObject(_root, "CarType", "DieselType");  // 柴油车
    cJSON_AddNumberToObject(_root, "GPS_Time", 10);   // 10s
    cJSON_AddNumberToObject(_root, "Car_Protocol", 0);   //
    cJSON_AddNumberToObject(_root, "CAN_bps", 500000);   // 500 kbps
    cJSON_AddNumberToObject(_root, "CAN_PIN", 0);

    out = cJSON_Print(_root);
    //out = cJSON_PrintUnformatted(_root);
    cJSON_Delete(_root);
    if(NULL==out)
    {
        printf("no memory, perused: %d  \n", mem_perused());  fflush(stdout);
        return -1;
    }
    //printf("data write to %s  \n", json_table);  fflush(stdout);
    //out = cJSON_Print(_root);
    printf("%s\n", out); fflush(stdout);
    fd = fopen(path, "w+");  // w+ : 可读可写, 可以不存在, 必会擦掉原有内容从头写
    if(NULL==fd)
    {
        printf("fopen fail!\n"); fflush(stdout);
        mem_free(out);
        fflush(stdout);
        return -2;
    }
    fwrite(out, strlen(out), 1, fd);
    fwrite("\n", 1, 1, fd);
    fflush(fd);
    fclose(fd);
    mem_free(out);
    printf("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    return 0;
}
#endif
//加密函数,iteration迭代次数
static void __tea_encrypt(uint32_t* v, const uint32_t* const key, const uint32_t _iteration)
{
	uint32_t v0 = v[0], v1 = v[1], sum = 0, i;           /* set up */
	uint32_t delta = 0x9e3779b9;                     /* a key schedule constant */
	uint32_t k0 = key[0], k1 = key[1], k2 = key[2], k3 = key[3];   /* cache key */
	for (i = 0; i < _iteration; i++)  // for (i = 0; i < 32; i++)
	{                       /* basic cycle start */
		sum += delta;
		v0 += ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
		v1 += ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
	}                                              /* end cycle */
	v[0] = v0; v[1] = v1;
}
uint32_t tea_encrypt(void* const _data, const uint32_t _dsize, const uint32_t* const key, const uint32_t _iteration)
{
	uint32_t dsize = 0;// _dsize;
	uint32_t* const data = (uint32_t*)_data;
	uint32_t index;
	uint32_t iteration;
	// 64bit/8byte对齐,未对齐部分补齐
	dsize = 0;
	if ((_dsize & 0x07) > 0) dsize = 8;
	dsize = dsize + (_dsize & 0xFFFFFFF8);// dsize = (dsize & 0xFFFFFFF8) + 8;
	//dsize = dsize >> 2;
	msg_print("[%s--] size[%d %d]\n", __func__, _dsize, dsize);
	// 迭代次数必须是32的倍数
	iteration = 0;
	if ((_iteration & 0x1F) > 0) iteration = 32;
	iteration = iteration + (_iteration & 0xFFFFFFE0);
	for (index = 0; index < (dsize >> 2); index+=2)
	{
		__tea_encrypt(&data[index], key, iteration);
	}
	print_hex((char*)data, dsize);
	return dsize;
}
//解密函数
static void __tea_decrypt(uint32_t* v, const uint32_t* const key, const uint32_t _iteration)
{
	uint32_t v0 = v[0], v1 = v[1], sum = 0xC6EF3720, i;  /* set up */
	uint32_t delta = 0x9e3779b9;                     /* a key schedule constant */
	uint32_t k0 = key[0], k1 = key[1], k2 = key[2], k3 = key[3];   /* cache key */
	for (i = 0; i < _iteration; i++)  // for (i = 0; i < 32; i++)
	{                         /* basic cycle start */
		v1 -= ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
		v0 -= ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
		sum -= delta;
	}                                              /* end cycle */
	v[0] = v0; v[1] = v1;
}
void tea_decrypt(void* const _data, const uint32_t _dsize, const uint32_t* const key, const uint32_t _iteration)
{
	uint32_t dsize = _dsize;
	uint32_t* const data = (uint32_t*)_data;
	uint32_t index;
	uint32_t iteration;
	// 64bit/8byte对齐,未对齐部分舍弃
	dsize = dsize & 0xFFFFFFF8;
	//dsize = dsize >> 2;
	msg_print("[%s--] size[%d %d]\n", __func__, _dsize, dsize);
	print_hex((char*)data, dsize);
	// 迭代次数必须是32的倍数
	iteration = 0;
	if ((_iteration & 0x1F) > 0) iteration = 32;
	iteration = iteration + (_iteration & 0xFFFFFFE0);
	for (index = 0; index < (dsize >> 2); index += 2)
	{
		__tea_decrypt(&data[index], key, iteration);
	}
}

/*
加密信息,把一串信息经多次加密后加密成密文,加密密文不可解密仅用于校验和鉴权
参数：
		_buf:编码后的数据缓存
		_bsize:缓存大小
		_id:需要加密的信息
		_idlen:加密信息长度>=4*6=24
		_key:加密的初始密钥
返回值:加密后数据的长度
校验和鉴权:将相同的信息以相同的方式加密，最后匹配输出的内容
*/
int encryp_id(void* const _buf, const int _bsize, const char _id[], const int _idlen, const uint32_t _key[4])
{
	int i;
	int len = 0;
	uint32_t dsize = 0;
	uint32_t  data[32];
	uint32_t key[4];
	char* const hex = (char*)_buf;
	memset(_buf, 0, _bsize);
	memset(data, 0, sizeof(data));
	memset(key, 0, sizeof(key));
	memcpy(data, _id, _idlen);
	memcpy(key, _key, sizeof(key));
	// 第一轮加密
	dsize = tea_encrypt(data, _idlen, key, 64);
	// 第二轮加密
	// 利用加密后的内容更新密钥
	for (i = 0; i < 4; i++)
	{
		key[i] = key[i] + data[i] + data[i] ^ data[i+1];
	}
	dsize = tea_encrypt(data, dsize, key, 64);
	// 转成HEX的ASCII码
	len = 0;
	for (i = 0; i < (dsize >>2); i++)
	{
		len = strlen(hex);
		snprintf(&hex[len], _bsize - len, "%08X ", data[i]);
	}
	//print_hex_buf((char*)_buf, _bsize, (char*)data, dsize);
	len = strlen(hex);
	//msg_print("[%s--] size[%d %d]\n", __func__, dsize, len);
	//print_hex((char*)data, dsize);
	//msg_fflush();
	return len;
}

/*
编码数据,用于把多项数据合并成一个JSON字符串
参数：
		_buf:编码后的数据缓存
		_bsize:缓存大小
		_sn:SN号
		_host:服务器IP/域名:端口,不使用时为:""
		_id:软件加密信息,转成HEX的 ASCII码.该功能通过烧号软件对代码进行加密,不使用时为:""
		_msg:附加消息.
*/

int encode_cfg_buffer(void* const _buf, const int _bsize, const char _sn[], const char _host[], const char _id[], const char _msg[])
{
	cJSON* _root = NULL;
	//cJSON *item_json = NULL;
	char* out = NULL;
	int _size;
	char* const buffer = (char*)_buf;

	_root = cJSON_CreateObject();
	cJSON_AddStringToObject(_root, "SN", _sn);     // 序列号
	if(strlen(_host)>0) cJSON_AddStringToObject(_root, "Host", _host); // 域名端口
	if (strlen(_id) > 0) cJSON_AddStringToObject(_root, "ID", _id); // 域名端口
	cJSON_AddStringToObject(_root, "ATT", _msg);   // 附加信息

	//out = cJSON_Print(_root);
	out = cJSON_PrintUnformatted(_root);
	cJSON_Delete(_root);
	if (NULL == out)
	{
		printf("no memory, perused: %d  \n", mem_perused());  fflush(stdout);
		return -1;
	}
	//printf("data write to %s  \n", json_table);  fflush(stdout);
	//out = cJSON_Print(_root);
	printf("%s\n", out); fflush(stdout);
	_size = strlen(out);
	if (_size >= _bsize)
	{
		mem_free(out);
		return -1;
	}
	memcpy(buffer, out, _size);
	buffer[_size++] = '\0';
	mem_free(out);
	printf("memory perused: %d  \n", mem_perused());  fflush(stdout);
	return _size;
}
// _item:"SN","Host","ATT"
int get_cfg_item(void* const _data, const int _dsize, const char _item[], const char _json[])
{
	cJSON* _root = NULL;
	cJSON *item_json = NULL;
	int _size;
	char* const data = (char*)_data;
	char* str_value = NULL;

	_root = cJSON_Parse((const char*)_json);
	if (NULL == _root)  // 数据损坏
	{
		//printf("data bad\n"); fflush(stdout);
		return -2;
	}
	item_json = cJSON_GetObjectItem(_root, _item);
	if (NULL == item_json)  // 数据损坏
	{
		goto json_bad;
	}
	str_value = cJSON_GetStringValue(item_json);
	if (NULL == str_value)  // 数据损坏
	{
		goto json_bad;
	}
	_size = strlen(str_value);

	memcpy(data, str_value, _size);
	data[_size++] = '\0';
	printf("memory perused: %d  \n", mem_perused());  fflush(stdout);
	cJSON_Delete(_root);
	return _size;
json_bad:
	//printf("item_json bad\n"); fflush(stdout);
	cJSON_Delete(_root);
	return -3;
}

// 编码本地配置
int encode_local_cfg(const struct load_local_cfg_data* const _local_cfg, const char path[], const char encrypt_path[]/*, const int _encrypt*/)
{
	cJSON* _root = NULL;
	char* out = NULL;
	int _size;

	_root = cJSON_CreateObject();
	cJSON_AddStringToObject(_root, LOCAL_CFG_NAME,   _local_cfg->Name);       // 产品名称
	cJSON_AddStringToObject(_root, LOCAL_CFG_DES,    _local_cfg->Des);        // 产品描述
	cJSON_AddStringToObject(_root, LOCAL_CFG_HOST,   _local_cfg->Host);       // IP/Port
	cJSON_AddStringToObject(_root, LOCAL_CFG_MODEL,  _local_cfg->Model);      // 产品型号,三个字符
	cJSON_AddStringToObject(_root, LOCAL_CFG_ENCODE, _local_cfg->Encode);     // 编码格式，码制
	cJSON_AddNumberToObject(_root, LOCAL_CFG_QUALITY,_local_cfg->Quality);    // 质检
	cJSON_AddNumberToObject(_root, LOCAL_CFG_DEVELOP, _local_cfg->Develop);   // 开发模式
    cJSON_AddNumberToObject(_root, LOCAL_CFG_ENCRYP,  _local_cfg->Encryp);    // 软件加密
    cJSON_AddNumberToObject(_root, LOCAL_CFG_KEY1,    _local_cfg->Key[0]);    // 密钥1
    cJSON_AddNumberToObject(_root, LOCAL_CFG_KEY2,    _local_cfg->Key[1]);    // 密钥2
    cJSON_AddNumberToObject(_root, LOCAL_CFG_KEY3,    _local_cfg->Key[2]);    // 密钥3
    cJSON_AddNumberToObject(_root, LOCAL_CFG_KEY4,    _local_cfg->Key[3]);    // 密钥4
    //cJSON_AddItemToObject(_root, LOCAL_CFG_KEY, cJSON_CreateIntArray(_local_cfg->Key, sizeof(_local_cfg->Key)/sizeof(_local_cfg->Key[0])));
	cJSON_AddStringToObject(_root, LOCAL_CFG_NAME".Des",   "产品名称");    
	cJSON_AddStringToObject(_root, LOCAL_CFG_DES".Des",    "产品描述");      
	cJSON_AddStringToObject(_root, LOCAL_CFG_HOST".Des",   "IP/Port");    
	cJSON_AddStringToObject(_root, LOCAL_CFG_MODEL".Des",  "产品型号");   
	cJSON_AddStringToObject(_root, LOCAL_CFG_ENCODE".Des", "SN码制,值：SN_NUMBER[数字码旧码]/SN_GENERAL[普码]/SN_SIMPLE[简码]/SN_FEATURE[特征码]");     
	cJSON_AddStringToObject(_root, LOCAL_CFG_QUALITY".Des", "质检,值：0[烧号]/1[质检]");
	cJSON_AddStringToObject(_root, LOCAL_CFG_DEVELOP".Des", "开发模式，值：0[关闭]/1[开启]");

	out = cJSON_Print(_root);
	//out = cJSON_PrintUnformatted(_root);
	cJSON_Delete(_root);
	if (NULL == out)
	{
		printf("no memory, perused: %d  \n", mem_perused());  fflush(stdout);
		return -1;
	}
	//printf("data write to %s  \n", json_table);  fflush(stdout);
	//out = cJSON_Print(_root);
	//printf("%s\n", out);
	_size = strlen(out);
	if (_size >= sizeof(json_buf))
	{
		mem_free(out);
		return -2;
	}
	memset(json_buf, 0, sizeof(json_buf));
	memcpy(json_buf, out, _size);
	json_buf[_size++] = '\n';
	mem_free(out);
	msg_print("memory perused: %d  \n", mem_perused());
	msg_print("[%s--] json[%d]:\n%s\n", __func__, _size, json_buf);
	write_to_file(path, json_buf, _size);
	//write_to_file("encode.cfg", json_buf, _size);
    if(NULL!=encrypt_path)//if(1==_encrypt)
    {
        //char encrypt_path[128];
        //memset(encrypt_path, 0, sizeof(encrypt_path));
        //snprintf(encrypt_path, sizeof (encrypt_path)-1, "%s.en", path);
        msg_print("[%s--] tea_encrypt\n", __func__);
        _size = tea_encrypt(json_buf, _size, tea_key, local_cfg_tea_iteration);
        msg_print("[%s--] write_to_file[%d]\n", __func__, _size);
        //write_to_file("encrypt", json_buf, _size);
        write_to_file(encrypt_path, json_buf, _size);
#if 0
        memset(json_buf, 0, sizeof(json_buf));
        _size = read_from_file("encrypt", json_buf, sizeof(json_buf));
        msg_print("[%s--] read_from_file[%d]\n", __func__, _size);
        tea_decrypt(json_buf, _size, tea_key, local_cfg_tea_iteration);
        _size = strlen(json_buf);
        msg_print("[%s--] tea_decrypt json[%d]:\n%s\n", __func__, _size, json_buf);
        write_to_file("encode_decrypt", json_buf, _size);
#endif
    }
	msg_fflush();
	return _size;
}
#if 0
struct encode_list {
	int value;        //  编码值
	char name[64];    //  名字
	//char des[64];     //  编码描述
};
int get_encode_format(struct load_local_cfg_data* const _local_cfg)
{
	static const struct encode_list _list[4] ={
		{SN_NUMBER,  "SN_NUMBER",  /*"数字码"*/},
		{SN_GENERAL, "SN_GENERAL", /*"普码"*/},
		{SN_SIMPLE,  "SN_SIMPLE",  /*"简码"*/},
		{SN_FEATURE, "SN_FEATURE", /*"特征码"*/},
	};
	//char des_gerenal[] = "数字码";     //  编码描述
	static const int _list_size = sizeof(_list) / sizeof(_list);
	int i;
	for (i = 0; i < _list_size; i++)
	{
		if (0 == strcmp(_local_cfg->Encode, _list[i].name))
		{
			_local_cfg->encode_format = _list[i].value;
			//memcpy(_local_cfg->encode_des, _list[i].des, strlen(_list[i].des));
			break;
		}
	}
#if 0
	memset(_local_cfg->encode_des, 0, sizeof(_local_cfg->encode_des));
	switch (_local_cfg->encode_format)
	{
	case SN_NUMBER:
		memcpy(_local_cfg->encode_des, des_gerenal, strlen("数字码"));
		break;
	case SN_GENERAL:
		memcpy(_local_cfg->encode_des, des_gerenal, strlen("普码"));
		break;
	case SN_SIMPLE:
		memcpy(_local_cfg->encode_des, des_gerenal, strlen("简码"));
		break;
	case SN_FEATURE:
		memcpy(_local_cfg->encode_des, des_gerenal, strlen("特征码"));
		break;
	default:
		break;
	}
#endif
	return 0;
}
#endif
struct encode_list {
    int value;        //  编码值
    char name[64];    //  名字
    //char des[64];     //  编码描述
};
#define sn_value_number   "SN_NUMBER"
#define sn_value_general  "SN_GENERAL"
#define sn_value_simple   "SN_SIMPLE"
#define sn_value_feature  "SN_FEATURE"
static const struct encode_list _list[4] ={
    {SN_NUMBER,  sn_value_number},
    {SN_GENERAL, sn_value_general},
    {SN_SIMPLE,  sn_value_simple},
    {SN_FEATURE, sn_value_feature},
};
static const int _list_size = sizeof(_list) / sizeof(_list[0]);
int get_encode_format(const char _Encode[])
{
    int encode_format;
    int i;
    encode_format = 0;
    for (i = 0; i < _list_size; i++)
    {
        if (0 == strcmp(_Encode, _list[i].name))
        {
            encode_format = _list[i].value;
            break;
        }
    }
    return encode_format;
}
int get_encode_format_value(const int _encode_format, char _value[], const uint32_t _vsize)
{
    int i;
    memset(_value, 0, _vsize);
    memcpy(_value, "SN_NULL", 7);
    for (i = 0; i < _list_size; i++)
    {
        if (_encode_format == _list[i].value)
        {
            memset(_value, 0, _vsize);
            memcpy(_value, _list[i].name, strlen(_list[i].name));
            break;
        }
    }
    return 0;
}

// 加载本地配置
int load_local_cfg(struct load_local_cfg_data* const _local_cfg, const char path[], const int _encrypt)
{
	cJSON* _root = NULL;
	cJSON* item_json = NULL;
	int _size;
	char* str_value = NULL;
	//FILE* fd = NULL;

	memset(_local_cfg, 0, sizeof(struct load_local_cfg_data));
	memset(msg_buf, 0, sizeof(msg_buf));
	msg_print("[%s--]\n", __func__);
	msg_fflush();
#if 0
	//if (0 != read_from_file(path, json_buf, sizeof(json_buf)))
	{
		/*
		char Name[64];           //  产品名称
		char Des[64];            //  产品描述
		char Host[64];           //  IP/Port
		char Model[4];           //  产品型号,三个字符
		char Encode[64];         //  编码格式，码制
		*/
		// 创建默认文件
		struct load_local_cfg_data _cfg = { "OBDII-4G", "车载在线诊断设备", "obd4g.zodtool.com:9911", "0A0", "SN_GENERAL", 0, 1 , ENCRYP_TEA , {0x37452908, 0x37DE2CA8, 0x3BCD5908, 0x375FFEA8} };
		printf("fopen %s fail!\n", path);
        encode_local_cfg(&_cfg, path, 0);
		//return -1;
	}
	msg_fflush(); //  write_to_file(cfg_log_path, msg_buf, strlen(msg_buf));
#endif
	memset(json_buf, 0, sizeof(json_buf));
	_size = read_from_file(path, json_buf, sizeof(json_buf));
	if (0 >= _size)
	{
		return -2;
	}
    if(1==_encrypt)
    {
        msg_print("[%s--] read_from_file[%d]\n", __func__, _size);
        tea_decrypt(json_buf, _size, tea_key, local_cfg_tea_iteration);
        _size = strlen(json_buf);
        msg_print("[%s--] tea_decrypt json[%d]:\n%s\n", __func__, _size, json_buf);
        write_to_file("load_decrypt", json_buf, _size);
    }
#if 0
	fd = fopen(path, "r");
	if (NULL == fd)
	{
		/*
		char Name[64];           //  产品名称
		char Des[64];            //  产品描述
		char Host[64];           //  IP/Port
		char Model[4];           //  产品型号,三个字符
		char Encode[64];         //  编码格式，码制
		*/
		// 创建默认文件
		struct load_local_cfg_data _cfg = { "OBDII-4G", "车载在线诊断设备", "obd4g.zodtool.com:9911", "0A0", "SN_GENERAL"};
		printf("fopen %s fail!\n", path); 
		encode_local_cfg(&_cfg, path);
		fd = fopen(path, "r");
		if (NULL == fd) return -1;
	}
	fseek(fd, 0, SEEK_END);
	_size = ftell(fd);
	if (_size > sizeof(json_buf))
	{
		fclose(fd);
		return -2;
	}
	fseek(fd, 0, SEEK_SET);
	printf("_size %s :%ld \n", path, _size);  fflush(stdout);
	memset(json_buf, 0, sizeof(json_buf));
	fread(json_buf, _size, 1, fd);
	fclose(fd);
#endif
	_root = cJSON_Parse((const char*)json_buf);
	if (NULL == _root)  // 数据损坏
	{
		//printf("data bad\n"); fflush(stdout);
		return -3;
	}
	item_json = cJSON_GetObjectItem(_root, LOCAL_CFG_NAME);
	if (NULL == item_json) goto json_bad; // 数据损坏
	str_value = cJSON_GetStringValue(item_json);
	if (NULL == str_value) goto json_bad; // 数据损坏
	memcpy(_local_cfg->Name, str_value, strlen(str_value));
	item_json = cJSON_GetObjectItem(_root, LOCAL_CFG_DES);
	if (NULL == item_json) goto json_bad; // 数据损坏
	str_value = cJSON_GetStringValue(item_json);
	if (NULL == str_value) goto json_bad; // 数据损坏
	memcpy(_local_cfg->Des, str_value, strlen(str_value));
	item_json = cJSON_GetObjectItem(_root, LOCAL_CFG_HOST);
	if (NULL == item_json) goto json_bad; // 数据损坏
	str_value = cJSON_GetStringValue(item_json);
	if (NULL == str_value) goto json_bad; // 数据损坏
	memcpy(_local_cfg->Host, str_value, strlen(str_value));
	item_json = cJSON_GetObjectItem(_root, LOCAL_CFG_MODEL);
	if (NULL == item_json) goto json_bad; // 数据损坏
	str_value = cJSON_GetStringValue(item_json);
	if (NULL == str_value) goto json_bad; // 数据损坏
	memcpy(_local_cfg->Model, str_value, strlen(str_value));
	item_json = cJSON_GetObjectItem(_root, LOCAL_CFG_ENCODE);
	if (NULL == item_json) goto json_bad; // 数据损坏
	str_value = cJSON_GetStringValue(item_json);
	if (NULL == str_value) goto json_bad; // 数据损坏
	memcpy(_local_cfg->Encode, str_value, strlen(str_value));
	//get_encode_format(_local_cfg);
	item_json = cJSON_GetObjectItem(_root, LOCAL_CFG_QUALITY);
	if (NULL == item_json) goto json_bad; // 数据损坏
	_local_cfg->Quality = item_json->valueint;
	item_json = cJSON_GetObjectItem(_root, LOCAL_CFG_DEVELOP);
	if (NULL == item_json) goto json_bad; // 数据损坏
	_local_cfg->Develop = item_json->valueint;
	item_json = cJSON_GetObjectItem(_root, LOCAL_CFG_ENCRYP);
	if (NULL == item_json) goto json_bad; // 数据损坏
    _local_cfg->Encryp = item_json->valueint;
	item_json = cJSON_GetObjectItem(_root, LOCAL_CFG_KEY1);
	if (NULL == item_json) goto json_bad; // 数据损坏
    _local_cfg->Key[0] = item_json->valueint;
	item_json = cJSON_GetObjectItem(_root, LOCAL_CFG_KEY2);
	if (NULL == item_json) goto json_bad; // 数据损坏
    _local_cfg->Key[1] = item_json->valueint;
	item_json = cJSON_GetObjectItem(_root, LOCAL_CFG_KEY3);
	if (NULL == item_json) goto json_bad; // 数据损坏
    _local_cfg->Key[2] = item_json->valueint;
	item_json = cJSON_GetObjectItem(_root, LOCAL_CFG_KEY4);
	if (NULL == item_json) goto json_bad; // 数据损坏
    _local_cfg->Key[3] = item_json->valueint;

	printf("memory perused: %d  \n", mem_perused());  fflush(stdout);
	cJSON_Delete(_root);
	msg_fflush();
	return 0;
json_bad:
	//printf("item_json bad\n"); fflush(stdout);
	cJSON_Delete(_root);
	return -4;
}
int decode_cfg_buffer(struct cfg_data* const cfg, uint8_t buffer[], const uint16_t bsize)
{
    cJSON *_root = NULL;
    cJSON *item_json = NULL;
    //char *out = NULL;
    //long _size=0;
    {
        
        _root = cJSON_Parse((const char *)buffer);
        if(NULL == _root)  // 数据损坏
        {
            //printf("data bad\n"); fflush(stdout);
            return -2;
        }
        item_json = cJSON_GetObjectItem(_root, "Host");
        if(NULL == item_json)  // 数据损坏
        {
            goto json_bad;
        }
        memset(cfg, 0, sizeof(struct cfg_data));
        memcpy(cfg->Host, item_json->valuestring, strlen(item_json->valuestring));
        item_json = cJSON_GetObjectItem(_root, "Port");
        if(NULL == item_json)  // 数据损坏
        {
            goto json_bad;
        }
        cfg->Port = item_json->valueint;
        item_json = cJSON_GetObjectItem(_root, "Debug");
        cfg->Debug = 0;
        if(NULL != item_json)  // 数据损坏
        {
            cfg->Debug = item_json->valueint;
        }
        item_json = cJSON_GetObjectItem(_root, "SLL");
        if(NULL == item_json)  // 数据损坏
        {
            goto json_bad;
        }
        cfg->ssl = item_json->valueint;
        item_json = cJSON_GetObjectItem(_root, "CarType");
        if(NULL == item_json)  // 数据损坏
        {
            goto json_bad;
        }
        memcpy(cfg->CarType, item_json->valuestring, strlen(item_json->valuestring));
        item_json = cJSON_GetObjectItem(_root, "GPS_Time");
        if(NULL == item_json)  // 数据损坏
        {
            goto json_bad;
        }
        cfg->GPS_Time = item_json->valueint;
        item_json = cJSON_GetObjectItem(_root, "Car_Protocol");
        if(NULL == item_json)  // 数据损坏
        {
            goto json_bad;
        }
        cfg->Car_Protocol = item_json->valueint;
        item_json = cJSON_GetObjectItem(_root, "CAN_bps");
        if(NULL == item_json)  // 数据损坏
        {
            goto json_bad;
        }
        cfg->CAN_bps = item_json->valueint;
        item_json = cJSON_GetObjectItem(_root, "CAN_PIN");
        if(NULL == item_json)  // 数据损坏
        {
            goto json_bad;
        }
        cfg->CAN_PIN = item_json->valueint;
        item_json = cJSON_GetObjectItem(_root, "CanBus");
        if(NULL == item_json)  // 数据损坏
        {
            goto json_bad;
        }
        cfg->CanBus = item_json->valueint;
    }
    cJSON_Delete(_root);
    //printf("memory perused: %d  \n", mem_perused());  fflush(stdout);
    //fflush(stdout);
    return 0;
    
json_bad:
            //printf("item_json bad\n"); fflush(stdout);
            cJSON_Delete(_root);
            return -3;
}
#if 0
int decode_cfg(struct cfg_data* const cfg, const char* const path, uint8_t buffer[], const uint16_t bsize)
{
    long _size=0;
    int fd = 0;
    
    memset(buffer, 0x00, bsize);
    fd = open(path, O_RDONLY, 0);
    if(0 > fd)
    {
        //pr_debug("file %s not exist! \n", filename);  fflush(stdout);
      return -1;
    }
    else
    {
        _size = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        if(_size>bsize)
        {
            close(fd);
            return -1;
        }
        //pr_debug("%s@%d _size:%ld\n", __func__, __LINE__, _size); fflush(stdout);
        // read
        read(fd, buffer, (size_t)_size);
        close(fd);
        
        return decode_cfg_buffer(cfg, buffer, bsize);
    }
}

int save_cfg(const struct cfg_data* const cfg, const char* const path, uint8_t buffer[], const uint16_t bsize)
{
    cJSON *_root = NULL;
    cJSON *item_json = NULL;
    char *out = NULL;
    int fd = 0;
    uint16_t _size=0;

    _root = cJSON_CreateObject();
    cJSON_AddStringToObject(_root, "cJSON Version", cJSON_Version());
    cJSON_AddStringToObject(_root, "Host", cfg->Host);
    cJSON_AddNumberToObject(_root, "Port", cfg->Port);
    if(1==cfg->Debug) cJSON_AddNumberToObject(_root, "Debug", cfg->Debug);
    cJSON_AddNumberToObject(_root, "SLL", cfg->ssl);
    cJSON_AddItemToObject(_root, "SLL.Des", item_json = cJSON_CreateObject());
    cJSON_AddStringToObject(item_json, "0", "SSL_INFO");
    cJSON_AddStringToObject(item_json, "1", "SSL_RSA");
    cJSON_AddStringToObject(item_json, "2", "SSL_SM2");
    cJSON_AddStringToObject(_root, "CarType", cfg->CarType);   // 汽油车
    cJSON_AddItemToObject(_root, "CarType.Des", item_json = cJSON_CreateObject());
    cJSON_AddStringToObject(item_json, "FuelType", "汽油车");
    cJSON_AddStringToObject(item_json, "DieselType", "柴油车");
    cJSON_AddNumberToObject(_root, "GPS_Time", cfg->GPS_Time);   // 10s
    cJSON_AddNumberToObject(_root, "Car_Protocol", cfg->Car_Protocol);   //
    cJSON_AddNumberToObject(_root, "CAN_bps", cfg->CAN_bps);   // 0 kbps
    cJSON_AddNumberToObject(_root, "CAN_PIN", cfg->CAN_PIN);
    cJSON_AddNumberToObject(_root, "CanBus", cfg->CanBus);
    cJSON_AddItemToObject(_root, "CanBus.Des", item_json = cJSON_CreateObject());
    cJSON_AddStringToObject(item_json, "0x02", "IDM_SAEJ1939");
    cJSON_AddStringToObject(item_json, "0x03", "IDM_KWP");
    cJSON_AddStringToObject(item_json, "0x04", "IDM_5BD");
    cJSON_AddStringToObject(item_json, "0x05", "IDM_ISO");
    cJSON_AddStringToObject(item_json, "0x06", "IDM_CAN_STD");
    cJSON_AddStringToObject(item_json, "0x07", "IDM_CAN_EXT");

    out = cJSON_Print(_root);
    //out = cJSON_PrintUnformatted(_root);
    cJSON_Delete(_root);
    if(NULL==out)
    {
        //printf("no memory, perused: %d  \n", mem_perused());  fflush(stdout);
        return -1;
    }
    _size = strlen(out)+1;
    if(_size>bsize) 
    {
        mem_free(out);
        close(fd);
        return -2;
    }
    memcpy(buffer, out, _size);
    //printf("data write to %s  \n", json_table);  fflush(stdout);
    //out = cJSON_Print(_root);
    //printf("%s\n", out); fflush(stdout);
    fd = open(path, O_WRONLY | O_TRUNC, 0);
    if(0 > fd)
    {
        //printf("fopen fail!\n"); fflush(stdout);
        mem_free(out);
        fflush(stdout);
        return -3;
    }
    _size = write(fd, out, (size_t)_size);
#if 0
    if(_size<0) 
    {
        mem_free(out);
        close(fd);
        return -4;
    }
#endif
    close(fd);
    mem_free(out);
    //printf("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    return 0;
}
/*
 * 数据交换函数，用于将 cfg中的 CAN_bps、CAN_PIN、Car_Protocol替换 path_src文件中的对应数据，同时备份到 path_des。该函数还会将 JSON数据备份到 buffer。
*/
int swap_cfg(struct cfg_data* const cfg, const char* const path_src, const char* const path_des, uint8_t buffer[], const uint16_t bsize)
{
    struct cfg_data* const file = (struct cfg_data*)buffer;
    const uint16_t _size = sizeof(struct cfg_data);
    memcpy(buffer, 0, bsize);
    if(0==decode_cfg(file, path_src, &buffer[_size], bsize-_size))
    {
        file->CAN_bps = cfg->CAN_bps;
        file->CAN_PIN = cfg->CAN_PIN;
        file->Car_Protocol = cfg->Car_Protocol;
        memcpy(cfg, file, sizeof(struct cfg_data));
        // JSON
        memcpy(buffer, 0, bsize);
        if(0==save_cfg(cfg, path_des, buffer, bsize))
        {
            return 0;
        }
        return -2;
    }
    return -1;
}
int update_cfg(struct cfg_data* const cfg, const char* const path_des, uint8_t buffer[], const uint16_t bsize)
{
    struct cfg_data* const file = (struct cfg_data*)buffer;
    const uint16_t _size = sizeof(struct cfg_data);
    memcpy(buffer, 0, bsize);
    if(0==decode_cfg(file, path_des, &buffer[_size], bsize-_size))
    {
        cfg->Car_Protocol = file->Car_Protocol;
        cfg->CAN_PIN = file->CAN_PIN;
        cfg->CAN_bps = file->CAN_bps;
        cfg->CanBus = file->CanBus;
        //memcpy(cfg, file, sizeof(struct cfg_data));
        // JSON
        memcpy(buffer, 0, bsize);
        if(0==save_cfg(cfg, path_des, buffer, bsize))
        {
            return 0;
        }
        return -2;
    }
    memcpy(buffer, 0, bsize);
    if(0==save_cfg(cfg, path_des, buffer, bsize))
    {
        return 0;
    }
    return -1;
}
#endif
