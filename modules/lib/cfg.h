/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : cfg.h
* Author             : Merafour
* Last Modified Date : 09/04/2019
* Description        : CFG.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#ifndef _CFG_H_
#define _CFG_H_

#ifdef __cplusplus
extern "C"
{
#endif
	
#include <stdint.h>
#include <stddef.h>
#ifndef NULL
#define NULL 0
#endif

	/*
	{
			"cJSON Version":        "1.7.12",
			"Host": "39.108.72.130",
			"Port": 9910,
			"SLL":  1,
			"SLL.Des":      {
					"0":    "SSL_INFO",
					"1":    "SSL_RSA",
					"2":    "SSL_SM2"
			},
			"CarType":      "FuelType",
			"CarType.Des":  {
					"FuelType":     "汽油车",
					"DieselType":   "柴油车"
			},
			"GPS_Time":     10,
			"Car_Protocol": 0,
			"CAN_bps":      0,
			"CAN_PIN":      0
	}
	*/
	struct cfg_data {
		char Host[64];           //  IP
		char CarType[16];        //  车辆类型
		uint16_t ssl;             //  加密方式
		uint16_t Port;            //  端口
		uint16_t Debug;           //  调试
		uint16_t GPS_Time;        //  数据上报时间
		uint16_t Car_Protocol;    //  汽车总线协议
		uint16_t CAN_PIN;         //  CAN PIN 引脚切换
		uint32_t CAN_bps;         //  CAN bps 波特率
		uint32_t CanBus;          //  CAN BUS CAN总线
	};
	struct load_local_cfg_data {
		char Name[64];           //  产品名称
		char Des[64];            //  产品描述
		char Host[64];           //  IP/Port
        char Model[32];           //  产品型号,三个字符
		char Encode[64];         //  编码格式，码制
		int Quality;             //  质检
		int Develop;             //  开发模式
        //int encode_format;       //  码值,不写入配置文件
		//char encode_des[64];     //  编码描述
        int Encryp;              //  软件加密
        int Key[4];              //  密钥
	};

#define ENCRYP_NULL           0         //  不使用软件加密
#define ENCRYP_TEA            1         //  使用TEA软件加密
#define LOCAL_CFG_NAME       "Name"     //  产品名称
#define LOCAL_CFG_DES        "Des"      //  产品描述
#define LOCAL_CFG_HOST       "Host"     //  IP/Port
#define LOCAL_CFG_MODEL      "Model"    //  产品型号,三个字符
#define LOCAL_CFG_ENCODE     "Encode"   //  编码格式，码制
#define LOCAL_CFG_QUALITY    "Quality"  //  质检
#define LOCAL_CFG_DEVELOP    "Develop"  //  开发模式
#define LOCAL_CFG_ENCRYP     "Encryp"   //  软件加密
#define LOCAL_CFG_KEY        "Key"     //  密钥
#define LOCAL_CFG_KEY1       "Key1"     //  密钥1
#define LOCAL_CFG_KEY2       "Key2"     //  密钥2
#define LOCAL_CFG_KEY3       "Key3"     //  密钥3
#define LOCAL_CFG_KEY4       "Key4"     //  密钥4

    extern int encode_local_cfg(const struct load_local_cfg_data* const _local_cfg, const char path[], const char encrypt_path[]/*, const int _encrypt*/);
    extern int load_local_cfg(struct load_local_cfg_data* const _local_cfg, const char path[], const int _encrypt);
	extern int write_to_file(const char path[], const char _data[], const uint32_t _dsize);
	extern int read_from_file(const char path[], char _data[], const uint32_t _dsize);
	extern int encode_cfg_buffer(void* const _buf, const int _bsize, const char _sn[], const char _host[], const char _id[], const char _msg[]);
	extern int encryp_id(void* const _buf, const int _bsize, const char _id[], const int _idlen, const uint32_t _key[4]);
    extern int get_encode_format(const char _Encode[]);
    extern int get_encode_format_value(const int _encode_format, char _value[], const uint32_t _vsize);


	extern const char local_cfg_path[];

#ifdef __cplusplus
}
#endif

#endif /* _CFG_H_ */

