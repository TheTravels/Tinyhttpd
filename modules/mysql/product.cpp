
/**
 * OBD串号写入指令
 */

#include "stdafx.h"
#include "VCI_III_SIDDlg.h"
#include "product.h"
#include "sql.h"
#include <string.h>
#include <memory.h>
//#include "mem_malloc.h"	
#include "cfg.h"

const uint32_t imodel_OBD_4G = IMODEL('0', 'A', '0'); // ('0'<<16)| ('A' << 8)| ('0' << 0);
const uint32_t imodel_OBD_4G_feature = IMODEL('Z', 'A', '0'); // ('Z' << 16) | ('A' << 8) | ('0' << 0);

int product_obj::analyse_serial_number(struct serial_number* _sn, CString strPid)
{
	char data[128];
	cstring_to_char(strPid, data, sizeof(data));
	return __analyse_serial_number(_sn, data, strlen(data));
}
int product_obj::analyse_serial_number_general(struct serial_number_general* _sn, CString strPid)
{
	char data[128];
	cstring_to_char(strPid, data, sizeof(data));
	return __analyse_serial_number_general(_sn, data, strlen(data));
}
int product_obj::get_serial_number_general(char* const _sn, CString strPid)
{
	char data[128];
	cstring_to_char(strPid, data, sizeof(data));
	return __get_serial_number_general(_sn, data, strlen(data));
}
int product_obj::__analyse_serial_number(struct serial_number* const _sn, const char* const data, const int _dsize)
{
	int i = 0;
	struct serial_number sn;
	uint8_t buffer[32];
	//const char* data = NULL;
	if (_dsize != 12) return -1;
	//data = strPid.GetBuffer(0);
	memset(buffer, 0, sizeof(buffer));
	memset(_sn, 0, sizeof(struct serial_number));
	for (i = 0; i < _dsize; i++)
	{
		buffer[i] = (uint8_t)data[i] & 0xFF;
	}
	for (i = 0; i < _dsize - 1; i++)
	{
		if (buffer[i] < '0' || buffer[i] > '9')
		{
			return -1;
		}
	}

	//strPid.ReleaseBuffer();
	sn.yearh = buffer[0];       // 年
	sn.batch[0] = buffer[1];    // 批次
	sn.batch[1] = buffer[2];
	sn.yearl = buffer[3];
	sn.month[0] = buffer[4];    // 月份
	sn.month[1] = buffer[5];
	sn.model[0] = buffer[6];    // 型号
	sn.model[1] = buffer[7];
	sn.number[0] = buffer[8];   // 序号/数量
	sn.number[1] = buffer[9];
	sn.number[2] = buffer[10];
	sn.fixed = buffer[11];      // 固定值

	sn.iyear = (sn.yearh - '0') * 10 + (sn.yearl - '0');         // 年
	sn.ibatch = (sn.batch[0] - '0') * 10 + (sn.batch[1] - '0');  // 批次
	sn.imonth = (sn.month[0] - '0') * 10 + (sn.month[1] - '0');  // 月份
	sn.imodel = (sn.model[0] - '0') * 10 + (sn.model[1] - '0');  // 型号
	sn.inumber = (sn.number[0] - '0') * 100 + (sn.number[1] - '0') * 10 + (sn.number[2] - '0');  // 序号/数量
	memcpy(_sn, &sn, sizeof(struct serial_number));
	// 对时间进行校验
	if ((0 == sn.imonth) || (sn.imonth > 12)) return -2;
	if (sn.iyear < 19) return -2;
	Log(false, _T("时间:%02d-%02d, 批次:%d, 型号:%d, 序号:%d, 固定码:%c\n"), sn.iyear, sn.imonth, sn.ibatch, sn.imodel, sn.inumber, sn.fixed);
	return 0;
}
// 校验编码
int check_encode(const char* const _code, const int _csize)
{
	// 码值
	static const char code_value[][2] =
	{
		{'0',0},{'1',1},{'2',2},{'3',3},{'4',4},{'5',5},{'6',6},{'7',7},{'8',8},{'9',9},
		{'A',1},{'B',2},{'C',3},{'D',4},{'E',5},{'F',6},{'G',7},{'H',8},
		{'I',0},{'J',1},{'K',2},{'L',3},{'M',4},{'N',5},{'O',6},{'P',7},{'Q',8},{'R',9},
		{'S',2},{'T',3},{'U',4},{'V',5},{'W',6},{'X',7},{'Y',8},{'Z',9}
	};
	static const unsigned int code_value_size = sizeof(code_value) / sizeof(code_value[0]);
	char code;
	int value;
	int i = 0, j = 0;
	for (i = 0; i < _csize; i++)
	{
		code = _code[i];
		value = -1;
		/*if (code >= 'a' && code <= 'z')  // 小写转大写
		{
			code = code - 'a' + 'A';
		}*/
		// VIN 码中不应该出现 I,O,Q 字符
		if (('I' == code) || ('O' == code) || ('Q' == code))
		{
			return -2;
		}
		for (j = 0; j < code_value_size; j++)
		{
			if (code == code_value[j][0])
			{
				value = j;
				break;
			}
		}
		// 非法字符
		if (-1 == value) return -3;
	}
	return 0;
}
// 校验区分码
int check_code(const char _code)
{
	// 码值
	static const char code_value[] = "0Z";
	static const unsigned int code_value_size = sizeof(code_value) - 1;
	char code;
	int value;
	unsigned int i = 0;
	value = -1;
	for (i = 0; i < code_value_size; i++)
	{
		code = _code;
		if (code == code_value[i])
		{
			value = i;
			break;
		}
	}
	// 非法字符
	if (-1 == value) return -3;
	return 0;
}
int product_obj::__analyse_serial_number_general(struct serial_number_general* const __sn, const char* const data, const int _dsize)
{
	// 码值
	static const char code_value[] = "0Z";
	static const unsigned int code_value_size = sizeof(code_value) - 1;
	static const char feature_list[][13] = {  // 特征值列表
		"ZA-----N----",
		"ZB-----N----",
	};
	static const int feature_list_size = sizeof(feature_list) / sizeof(feature_list[0]);
	static const char year_code[][2] =
	{
		{'0',0},{'1',1},{'2',2},{'3',3},{'4',4},{'5',5},{'6',6},{'7',7},{'8',8},{'9',9},
		{'A',10},{'B',11},{'C',12},{'D',13},{'E',14},{'F',15},{'G',16},{'H',17},
		{'I','-'},{'J',18},{'K',19},{'L',20},{'M',21},{'N',22},{'O','-'},{'P',23},{'Q','-'},{'R',24},
		{'S',25},{'T',26},{'U',27},{'V',28},{'W',29},{'X',30},{'Y',31},{'Z',32}
	};
	static const int year_code_size = sizeof(year_code) / sizeof(year_code[0]);
	static const char month_code[][2] =
	{
		{'0','-'},{'1',1},{'2',2},{'3',3},{'4',4},{'5',5},{'6',6},{'7',7},{'8',8},{'9',9},
		{'A',10},{'B',11},{'C',12},{'D',1},{'E',2},{'F',3},{'G',4},{'H',5},
		{'I','-'},{'J',6},{'K',7},{'L',8},{'M',9},{'N',10},{'O','-'},{'P',11},{'Q','-'},{'R',12},
		{'S',1},{'T',3},{'U',5},{'V',7},{'W',9},{'X',11},{'Y','-'},{'Z','-'}
	};
	static const int month_code_size = sizeof(year_code) / sizeof(year_code[0]);
	int i = 0, j = 0;
	struct serial_number_general sn;
	uint8_t buffer[32];
	const char* _code = NULL;
	int _code_size = 0;
	//char code;

	memset(buffer, 0, sizeof(buffer));
	memset(__sn, 0, sizeof(struct serial_number_general));
	memset(&sn, 0, sizeof(struct serial_number_general));
	// 特殊编码检查
	_code = data;
	_code_size = _dsize;
	for (i = 0; i < feature_list_size; i++)
	{
		for (j = 0; j < _dsize; j++)
		{
			if ((feature_list[i][0] == (data[j] & 0xFF)) && (feature_list[i][1] == (data[j+1] & 0xFF)) && (feature_list[i][7] == (data[j + 7] & 0xFF)))
			{
				sn.type = SN_FEATURE; // 特征码
				_code = &data[j];
				_code_size = _dsize-j;
				break;
			}
		}
	}
	// 拷贝编码
	for (i = 0; i < 12; i++)
	{
		buffer[i] = (uint8_t)_code[i] & 0xFF;
	}
	if ((SN_FEATURE != sn.type) && (12==_code_size)) // 普码
	{
		sn.type = SN_GENERAL;
	}
	else if (8 == _code_size) // 简码
	{
		sn.type = SN_SIMPLE;
	}
	else if (12 > _code_size) // ERROR
	{
		return -1;
	}
	if ((SN_GENERAL == sn.type) || (SN_FEATURE == sn.type)) // 普码/特征码
	{
		if (check_encode(_code, 12) < 0) return -2; // 校验编码字符 
		sn.code   = buffer[0];     // 区分码
		sn.model1 = buffer[1];     // 型号1
		sn.model2 = buffer[2];     // 型号2
		sn.area   = buffer[3];     // 地区
		sn.year   = buffer[4];     // 年份
		sn.month  = buffer[5];     // 月份
		sn.ver    = buffer[6];     // 版本
		sn.crc    = buffer[7];     // 校验码
		sn.batch  = buffer[8];     // 批号
		sn.number[0] = buffer[9];  // 序号/数量
		sn.number[1] = buffer[10]; // 序号/数量
		sn.number[2] = buffer[11]; // 序号/数量
	}
	if (SN_SIMPLE == sn.type) // 简码
	{
		if (check_encode(_code, 8) < 0) return -2; // 校验编码啊字符 
		sn.code = '0';              // 区分码
		sn.model1 = buffer[0];    // 型号1
		sn.model2 = buffer[1];    // 型号2
		sn.area = 'C';            // 中国大陆
		sn.year = buffer[2];      // 年份
		sn.month = buffer[3];     // 月份
		sn.ver = buffer[4];       // 版本
		sn.crc = 'N';             // 校验码
		sn.batch = buffer[5];     // 批号
		sn.number[0] = '0';       // 序号/数量
		sn.number[1] = buffer[6]; // 序号/数量
		sn.number[2] = buffer[7]; // 序号/数量
	}
	//if('1'== sn.code) return -2; // 区分码1为旧码
	if (check_code(sn.code) < 0) // 校验区分码
	{
		if ((12 == _dsize) && ('1' == data[0]))
		{
			/*log(false, false, "序列号：[");
			for(i=0; i<12; i++) log(false, false, "%c", data[i]);
			log(false, false, "] \n");*/
			Log(false, "序列号：[%c%c%c%c%c%c%c%c%c%c%c%c] \n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11]);
			Log(false, "编码方式为数字码/旧码，区分码[%c] \n", data[0]);
		}
		return -2; 
	}
	if ((SN_GENERAL == sn.type) || (SN_FEATURE == sn.type)) // 普码/特征码
	{
		Log(false, "序列号：[%c%c%c%c%c%c%c%c%c%c%c%c] \n", _code[0], _code[1], _code[2], _code[3], _code[4], _code[5], _code[6], _code[7], _code[8], _code[9], _code[10], _code[11]);
		/*log(false, false, "序列号：[");
		for (i = 0; i < 12; i++) log(false, false, "%c", _code[i]);
		log(false, false, "] \n");*/
	}
	if (SN_SIMPLE == sn.type) // 简码
	{
		Log(false, "序列号：[%c%c%c%c%c%c%c%c] \n", _code[0], _code[1], _code[2], _code[3], _code[4], _code[5], _code[6], _code[7]);
		/*log(false, false, "序列号：[");
		for (i = 0; i < 8; i++) log(false, false, "%c", _code[i]);
		log(false, false, "] \n");*/
	}
	
	if (SN_GENERAL == sn.type)  Log(false, "编码方式为普码，区分码[%c] \n", sn.code);
	if (SN_FEATURE == sn.type)  Log(false, "编码方式为特征码，区分码[%c] \n", sn.code);
	if (SN_SIMPLE == sn.type)  Log(false, "编码方式为简码，区分码[%c] \n", sn.code);
	// 计算年份
	for (i = 0; i < year_code_size; i++)
	{
		if (year_code[i][0] == sn.year)
		{
			if ('-' == year_code[i][0]) return -3; // 非法字符
			sn.iyear = 2000 + year_code[i][1];
			break;
		}
	}
	// 计算月份
	for (i = 0; i < month_code_size; i++)
	{
		if (month_code[i][0] == sn.month)
		{
			if ('-' == month_code[i][0]) return -3; // 非法字符
			sn.imonth = month_code[i][1];
			break;
		}
	}
	// 计算批次
	for (i = 0; i < year_code_size; i++)
	{
		if (year_code[i][0] == sn.batch)
		{
			if ('-' == year_code[i][0]) return -3; // 非法字符
			sn.ibatch = year_code[i][1];
			break;
		}
	}
	// 计算型号, 合成24位型号代码
	/*sn.imodel = sn.code;// &0xFF;
	Log(false, _T("imodel:0x%08X  %d\n"), sn.imodel, sn.imodel);
	sn.imodel = (sn.imodel << 8) | (sn.model1);
	Log(false, _T("imodel:0x%08X  %d\n"), sn.imodel, sn.imodel);
	sn.imodel = (sn.imodel << 8) | (sn.model2);
	Log(false, _T("imodel:0x%08X  %d\n"), sn.imodel, sn.imodel);*/
	sn.imodel = (sn.code << 16) | (sn.model1 << 8) | (sn.model2 << 0);
	sn.inumber = (sn.number[0] - '0') * 100 + (sn.number[1] - '0') * 10 + (sn.number[2] - '0');  // 序号/数量
	memcpy(__sn, &sn, sizeof(struct serial_number_general));
	// 对时间进行校验
	if ((0 == sn.imonth) || (sn.imonth > 12)) return -4;
	if (sn.iyear < 2019) return -5;
	//Log(false, _T("时间:%04d-%02d, 批次:%d, 型号:%c%c%c, 地区:%c, 序号:%d\n"), sn.iyear, sn.imonth, sn.ibatch, sn.code, sn.model1, sn.model2, sn.area, sn.inumber);
	Log(false, _T("时间:%04d-%02d, 型号:%c%c%c, 版本:%c, 校验:%c, 地区:%c, 批次:%d, 序号:%d\n"), sn.iyear, sn.imonth, sn.code, sn.model1, sn.model2, sn.ver, sn.crc, sn.area, sn.ibatch, sn.inumber);
	return 0;
}
int product_obj::__get_serial_number_general(char* const __sn, const char* const data, const int _dsize)
{
	// 码值
	static const char code_value[] = "0Z";
	static const unsigned int code_value_size = sizeof(code_value) - 1;
	static const char feature_list[][13] = {  // 特征值列表
		"ZA-----N----",
		"ZB-----N----",
	};
	static const int feature_list_size = sizeof(feature_list) / sizeof(feature_list[0]);
	static const char year_code[][2] =
	{
		{'0',0},{'1',1},{'2',2},{'3',3},{'4',4},{'5',5},{'6',6},{'7',7},{'8',8},{'9',9},
		{'A',10},{'B',11},{'C',12},{'D',13},{'E',14},{'F',15},{'G',16},{'H',17},
		{'I','-'},{'J',18},{'K',19},{'L',20},{'M',21},{'N',22},{'O','-'},{'P',23},{'Q','-'},{'R',24},
		{'S',25},{'T',26},{'U',27},{'V',28},{'W',29},{'X',30},{'Y',31},{'Z',32}
	};
	static const int year_code_size = sizeof(year_code) / sizeof(year_code[0]);
	static const char month_code[][2] =
	{
		{'0','-'},{'1',1},{'2',2},{'3',3},{'4',4},{'5',5},{'6',6},{'7',7},{'8',8},{'9',9},
		{'A',10},{'B',11},{'C',12},{'D',1},{'E',2},{'F',3},{'G',4},{'H',5},
		{'I','-'},{'J',6},{'K',7},{'L',8},{'M',9},{'N',10},{'O','-'},{'P',11},{'Q','-'},{'R',12},
		{'S',1},{'T',3},{'U',5},{'V',7},{'W',9},{'X',11},{'Y','-'},{'Z','-'}
	};
	static const int month_code_size = sizeof(year_code) / sizeof(year_code[0]);
	int i = 0, j = 0;
	struct serial_number_general sn;
	uint8_t buffer[32];
	const char* _code = NULL;
	int _code_size = 0;
	//char code;

	memset(buffer, 0, sizeof(buffer));
	memset(&sn, 0, sizeof(struct serial_number_general));
	// 特殊编码检查
	_code = data;
	_code_size = _dsize;
	for (i = 0; i < feature_list_size; i++)
	{
		for (j = 0; j < _dsize; j++)
		{
			if ((feature_list[i][0] == (data[j] & 0xFF)) && (feature_list[i][1] == (data[j + 1] & 0xFF)) && (feature_list[i][7] == (data[j + 7] & 0xFF)))
			{
				sn.type = SN_FEATURE; // 特征码
				_code = &data[j];
				_code_size = _dsize - j;
				break;
			}
		}
	}
	// 拷贝编码
	for (i = 0; i < 12; i++)
	{
		buffer[i] = (uint8_t)_code[i] & 0xFF;
	}
	if ((SN_FEATURE != sn.type) && (12 == _code_size)) // 普码
	{
		sn.type = SN_GENERAL;
	}
	else if (8 == _code_size) // 简码
	{
		sn.type = SN_SIMPLE;
	}
	else if (12 > _code_size) // ERROR
	{
		return -1;
	}
	if ((SN_GENERAL == sn.type) || (SN_FEATURE == sn.type)) // 普码/特征码
	{
		if (check_encode(_code, 12) < 0) return -2; // 校验编码字符 
		sn.code = buffer[0];     // 区分码
		sn.model1 = buffer[1];     // 型号1
		sn.model2 = buffer[2];     // 型号2
		sn.area = buffer[3];     // 地区
		sn.year = buffer[4];     // 年份
		sn.month = buffer[5];     // 月份
		sn.ver = buffer[6];     // 版本
		sn.crc = buffer[7];     // 校验码
		sn.batch = buffer[8];     // 批号
		sn.number[0] = buffer[9];  // 序号/数量
		sn.number[1] = buffer[10]; // 序号/数量
		sn.number[2] = buffer[11]; // 序号/数量
	}
	if (SN_SIMPLE == sn.type) // 简码
	{
		if (check_encode(_code, 8) < 0) return -2; // 校验编码啊字符 
		sn.code = '0';              // 区分码
		sn.model1 = buffer[0];    // 型号1
		sn.model2 = buffer[1];    // 型号2
		sn.area = 'C';            // 中国大陆
		sn.year = buffer[2];      // 年份
		sn.month = buffer[3];     // 月份
		sn.ver = buffer[4];       // 版本
		sn.crc = 'N';             // 校验码
		sn.batch = buffer[5];     // 批号
		sn.number[0] = '0';       // 序号/数量
		sn.number[1] = buffer[6]; // 序号/数量
		sn.number[2] = buffer[7]; // 序号/数量
	}
	if (check_code(sn.code) < 0) // 校验区分码
	{
		if ((12 == _dsize) && ('1' == data[0]))
		{
			memcpy(__sn, data, _dsize);
			return 0;
		}
		return -2;
	}
	if ((SN_GENERAL == sn.type) || (SN_FEATURE == sn.type)) // 普码/特征码
	{
		memcpy(__sn, _code, 12);
		return 0;
	}
	if (SN_SIMPLE == sn.type) // 简码
	{
		memcpy(__sn, _code, 8);
		return 0;
	}
	return -3;
}

int product_obj::program_device(const char __sn[], const char _host[], const int _quality, const int _develop, const int _encryp, const int _key[])
{
	char host[128];
	int port;
	if (strlen(_host) > 0)
	{
		memset(host, 0, sizeof(host));
		port = analysis_host(_host, host);
		Log(true, "domain:%s HOST:%s PORT:%d\n", _host, host, port);
		if (port < 128)
		{
			Log(true, "HOST Error!\n");
			return -1;
		}
	}
	char _sn[128];
	char _att[128];
	const uint8_t cmd_list[5] = {
		orders_obd::CMD_CONNECT,
		orders_obd::CMD_SWAP,
		orders_obd::CMD_RID,
		//orders_obd::CMD_WSN,
		//orders_obd::CMD_RSN,
		orders_obd::CMD_WJSON,
		orders_obd::CMD_RJSON,
	};
	const uint8_t cmd_list_quality[5] = {
		orders_obd::CMD_CONNECT,
		orders_obd::CMD_SWAP,
		orders_obd::CMD_RID,
		orders_obd::CMD_RSN,
	};
	const uint8_t cmd_list_size = sizeof(cmd_list) / sizeof(cmd_list[0]);
	const uint8_t cmd_list_size_quality = sizeof(cmd_list_quality) / sizeof(cmd_list_quality[0]);
	memset(&_sn, 0, sizeof(_sn));
	memset(&_att, 0, sizeof(_att));
	memcpy(_att, __sn, strlen(__sn));
	if (0 != __get_serial_number_general(_sn, __sn, strlen(__sn)))
	{
		Log(false, "序列号获取错误 [%s]\n", _sn);
		return -1;
	}
	memset(host, 0, sizeof(host));
	if(_develop) memcpy(host, _host, strlen(_host));
	if (0 == _quality)
	{
		return program_orders(cmd_list, cmd_list_size, _sn, host, _att, _encryp, _key, orders_obd::CMD_RJSON);
	}
	else
	{
		return program_orders(cmd_list_quality, cmd_list_size_quality, _sn, host, _att, _encryp, _key, orders_obd::CMD_RJSON);
	}
}
#include<stdlib.h>
//static const uint32_t encryp_key[4] = { 0x37ABFD08, 0x3CCE2268, 0x369DFF08, 0xBE5F56A8 };
int product_obj::program_orders(const uint8_t cmd_list[], const uint8_t cmd_list_size, const char _sn[], const char _host[], const char _att[], const int _encryp, const int _key[], const uint8_t _sql_cmd)
{
	int _quality = 0;
	time_t timer = time(NULL);
	orders_obd order;
	uint8_t pack[512];
	uint8_t buf[512];
	char uid[32]="12345678901234567890ABCDEF";
	uint8_t json_buf[512];
	char encryp[128];
	char hard_id[128];
	//char wsn[128];
	uint8_t check[12];
	int len, _size, _rsize;
	uint8_t count = 0;
	uint8_t cmd = 0;
	//CString str;
	uint8_t safe = 0x33; // ((timer >> 8) & 0xFF) + (timer & 0xFF);
	uint8_t index = 0;
	int ret = 0;
	uint8_t cmd_index;
	uint8_t _order;

	memset(hard_id, 0, sizeof(hard_id));
	timer = time(NULL);
	srand(timer);
	safe = rand() & 0xFF;
	cmd_index = 0;
	//timeout = 0;
	_size = 0;
	for (cmd_index = 0; cmd_index < cmd_list_size; cmd_index++)
	{
		memset(pack, 0, sizeof(pack));
		_order = cmd_list[cmd_index];
		if ((1 == _quality) && ((orders_obd::CMD_WSN == _order) || (orders_obd::CMD_WHOST == _order)))
		{
			continue;
		}
		_size = 12;
		_rsize = 0;
		switch (_order)
		{
		case orders_obd::CMD_CONNECT:
			//connect:
			memset(buf, 0, sizeof(buf));
			// CMD_CONNECT
			timer = time(NULL);
			srand(timer);
			for (count = 0; count < 12; count++)  // 发送连接请求，下位机要连续收到超过 10 连接包才准许连接,连接命令下位机不返回信息
			{
				buf[count] = rand() & 0xFF;                   // 随机数，连接设备的时候必须提供
				Sleep(10);  // 10 ms
			}
			buf[1] = buf[0] ^ safe;    // 安全码，除连接过程数据使用安全码做异或操作
			memcpy(check, buf, 12);
			ShowText(_T("正在连接设备 ... "), true, false);
			break;
		case orders_obd::CMD_SWAP:
			// CMD_SWAP
			for (count = 0; count < 6; count++)  // 生成校验信息发送给设备
			{
				buf[count] = buf[count] ^ safe;
			}
			ShowText(_T("正在与设备交换信息 ... "), true, false);
			break;
		case orders_obd::CMD_RID:
			memset(buf, 0, sizeof(buf));
			ShowText(_T("正在读取设备ID ... "), true, false);
			break;
		case orders_obd::CMD_WSN:
			memset(buf, 0, sizeof(buf));
			// CMD_WSN:
			memcpy(buf, _sn, 12);
			ShowText(_T("正在刷写序列号 ... "), true, false);
			break;
		case orders_obd::CMD_RSN:
			memset(buf, 0, sizeof(buf));
			ShowText(_T("正在回读序列号 ... "), true, false);
			break;
		case orders_obd::CMD_WHOST:
			memset(buf, 0, sizeof(buf));
			// CMD_WHOST:
			_size = strlen(_host);
			memcpy(buf, _host, _size);
			ShowText(_T("正在刷写域名端口 ... "), true, false);
			break;
		case orders_obd::CMD_RHOST:
			memset(buf, 0, sizeof(buf));
			_rsize = strlen(_host) + 32;
			ShowText(_T("正在回读域名端口 ... "), true, false);
			break;
		case orders_obd::CMD_WJSON:
			/*
			|   序列号   -       域名      :端口|
			"102905420108-obd4g.zodtool.com:9910"
			*/
			memset(buf, 0, sizeof(buf));
			memset(json_buf, 0, sizeof(json_buf));
			memset(encryp, 0, sizeof(encryp));
#if 0
			_size = strlen(_sn);
			memcpy(buf, _sn, _size);
			buf[_size++] = '-';
			memcpy(&buf[_size], _host, strlen(_host));
			_size += strlen(_host);
#endif
			//encryp_id(encryp, sizeof(encryp), uid, 24, encryp_key);
			if(1==_encryp) encryp_id(encryp, sizeof(encryp), uid, 24, (uint32_t*)_key);
			_size = encode_cfg_buffer(buf, 256, _sn, _host, encryp, _att);
			//ShowText(_T("正在写批量数据 ... "), true, false);
			//Log(false, _T("写入数据[%s]"), buf);
			//Log(false, _T("encryp[%d] :%s"), strlen(encryp), encryp);
			Log(false, _T("数据长度:encryp[%d] buf[%d]"), strlen(encryp), _size);
			Log(false, _T("正在写批量数据[%s] ... "), buf);
			_rsize = 256;
			memcpy(json_buf, buf, strlen((char*)buf));
			break;
		case orders_obd::CMD_RJSON:
			memset(buf, 0, sizeof(buf));
			_size = 12;
			_rsize = 256;
			ShowText(_T("正在读批量数据 ... "), true, false);
			break;
		default:
			break;
		}
		//ret = SN_Write_OBD_Send(pDlg, _T("正在连接设备 ... "), order, orders_obd::CMD_CONNECT, buf, 12, pack, sizeof(pack));
		len = order.encode(_order, buf, _size, pack, sizeof(pack));
		if (!Send(pack, len))
		{
			return -1;
		}
		if (0 == _rsize) _rsize = len;
		if ((!Recv(pack, _rsize))) //if ((!Recv(pack, len)))
		{
			return -2;
		}
		// 解码
		memset(buf, 0, sizeof(buf));
		len = order.decode(&cmd, pack, sizeof(pack), buf, sizeof(buf));
		if ((orders_obd::CMD_ACK == cmd) && (12 == len))
		{
			ShowText(_T("设备响应错误，请重试！"), false);
			//this->exit();
			return -3;
		}
		if (((0x88 | _order) != cmd) || (len < 2))
		{
			ShowText(_T("连接设备失败！请检查设备是否正常工作"), false);
			//this->exit();
			return -4;
		}

		switch (_order)
		{
		case orders_obd::CMD_CONNECT:
			for (count = 6; count < 12; count++)  // 校验设备返回信息
			{
				if (buf[count] != (check[count] ^ safe))
				{
					return -5;
				}
			}
			ShowText(_T("成功"), false);
			break;
		case orders_obd::CMD_SWAP:
			ShowText(_T("成功"), false);
			break;
		case orders_obd::CMD_RID:
			Log(false, _T("成功[ %02X  %02X  %02X  %02X %02X  %02X  %02X  %02X %02X  %02X  %02X  %02X]"), buf[0], buf[1], buf[2], buf[3]\
				, buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11]);
			sprintf(hard_id, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ", buf[0], buf[1], buf[2], buf[3]\
				, buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11]);
			//ShowText(tmp, false);
			memset(uid, 0, sizeof(uid));
			memcpy(uid, buf, 12);
			memcpy(&uid[12], buf, 12);
			break;
		case orders_obd::CMD_WSN:
			//tmp.Format(_T("成功[%s]"), strPid.GetBuffer(0));
			Log(false, _T("成功[%s]"), _sn);
			//strPid.ReleaseBuffer();
			//ShowText(tmp, false);
			break;
		case orders_obd::CMD_RSN:
			//if (strPid != ByteArray2CString(buf, 12))
			if (0 != strncmp(_sn, (char*)buf, 12))
			{
				Log(false, _T("失败(回读不匹配)"));
				//ShowText(tmp, false);
				Log(false, _T("条码[%s]"), _sn);
				//strPid.ReleaseBuffer();
				//ShowText(tmp, false);
				Log(false, _T("设备[%s]"), buf);
				//ShowText(tmp, false);
			}
			else
			{
				Log(false, _T("成功[%s]"), buf);
			}
			break;
		case orders_obd::CMD_WHOST:
			Log(false, _T("成功[%s]"), _host);
			break;
		case orders_obd::CMD_RHOST:
			if (0 != strncmp(_host, (char*)buf, strlen(_host)))
			{
				Log(false, _T("失败(回读不匹配)"));
				//ShowText(tmp, false);
				Log(false, _T("HOST[%s]\n"), _host);
				//ShowText(tmp, false);
				Log(false, _T("设备[%s]\n"), buf);
				//ShowText(tmp, false);
			}
			else
			{
				Log(false, _T("成功[%s]"), buf);
				//ShowText(tmp, false);
			}
			break;
		case orders_obd::CMD_WJSON:
			Log(false, _T("成功"));
			break;
		case orders_obd::CMD_RJSON:
			memset(pack, 0, sizeof(pack));
			_size = encode_cfg_buffer(pack, 256, _sn, _host, encryp, _att);
			if (0 != strncmp((char*)pack, (char*)buf, strlen((char*)pack)))
			{
				Log(false, _T("失败(回读不匹配)"));
				//ShowText(tmp, false);
				//Log(false, _T("HOST[%s]\n"), _host);
				Log(false, _T("写入[%s]\n"), pack);
				//ShowText(tmp, false);
				Log(false, _T("读出[%s]\n"), buf);
				//ShowText(tmp, false);
			}
			else
			{
				Log(false, _T("成功[%s]"), buf);
				//ShowText(tmp, false);
			}
			break;
		default:
			break;
		}
	}
	//if (_sql_cmd == _order)
	{
		msg(_T("\r\n正在写入数据库...\r\n"));
		if (0 == sql_insert_sn(hard_id, _sn))
		{
			msg(_T("写入数据库成功\r\n"));
		}
		else
		{
			msg(_T("写入数据库失败\r\n"));
		}
	}
	msg("*****全部完成*****\r\n");

	//this->exit();
	return 0;
}

void product_obj::ShowText(CString strText, bool bTime, bool bReturn)
{
	Dlg.ShowText(strText, bTime, bReturn);
}
bool product_obj::Send(const void* const nBuff, const uint16_t nLen)
{
	//if (!Dlg.SendCmd_Simple(nBuff, nLen))
	if (nLen!=uart->send(nBuff, nLen))
	//if (!uart->SendCmd_Simple((BYTE*)nBuff, nLen))
	{
		ShowText(_T("失败"), false);
		Dlg.m_EditShow.ReplaceSel(_T("1).检查USB线路是否有松动或异常。\r\n"));
		Dlg.m_EditShow.ReplaceSel(_T("2).USB上电或外部上电的电压是否正常。\r\n"));
		//Dlg.PidThread = NULL;
		//Dlg.ComClose();
		return false;
	}
	return true;
}
#if 0
bool product_obj::Recv(BYTE* nBuff, const DWORD nLen, DWORD dwTimeout)
{
	if ((!Dlg.RecvCmd_Simple(nBuff, nLen, dwTimeout)))
	{
		ShowText(_T("失败(接收)"), false);
		Dlg.m_EditShow.ReplaceSel(_T("1).检查USB线路是否有松动或异常。\r\n"));
		Dlg.m_EditShow.ReplaceSel(_T("2).USB上电或外部上电的电压是否正常。\r\n"));
		Dlg.PidThread = NULL;
		Dlg.ComClose();
		return false;
	}
	return true;
}
#endif
bool product_obj::Recv(void* const nBuff, const uint16_t nLen, DWORD dwTimeout)
{
#if 0
	memset(nBuff, 0, nLen);
	//Dlg.serial_recv(this, nBuff, nLen, dwTimeout);
	Dlg.RecvCmd_Simple(nBuff, nLen, dwTimeout);
	if (0!= pack_check((char*)nBuff, nLen))
	{
		ShowText(_T("失败(接收)"), false);
		Dlg.m_EditShow.ReplaceSel(_T("1).检查USB线路是否有松动或异常。\r\n"));
		Dlg.m_EditShow.ReplaceSel(_T("2).USB上电或外部上电的电压是否正常。\r\n"));
		Dlg.PidThread = NULL;
		Dlg.ComClose();
		return false;
	}
	return true;
#else
	int len = 0, _size = 0;
	unsigned int delay = 0;
	char* const data = (char* const)nBuff;
	memset(nBuff, 0, nLen);
	len = 0;
	for (delay = 0; delay < dwTimeout; delay += 100)
	{
		//_size = Dlg.serial_recv(this, &nBuff[len], nLen- len, 100);
		_size = uart->recv(&data[len], nLen - len, 100);
		len += _size;
		if (0 == pack_check((char*)nBuff, len))
		{
			return true;
		}
	}
	Log(false, _T("失败(接收):nLen[%d] len[%d]"), nLen, len);
	//ShowText(_T("失败(接收)"), false);
	Dlg.m_EditShow.ReplaceSel(_T("1).检查USB线路是否有松动或异常。\r\n"));
	Dlg.m_EditShow.ReplaceSel(_T("2).USB上电或外部上电的电压是否正常。\r\n"));
	//Dlg.PidThread = NULL;
	//Dlg.ComClose();
	return false;
#endif
}

void product_obj::Log(bool bTime, const char* fmt, ...)
{
	static char log_buf[512] = { 0 };
	va_list args;
	size_t length;
	memset(log_buf, 0, sizeof(log_buf));
	va_start(args, fmt);
	length = vsnprintf(log_buf, sizeof(log_buf) - 1, fmt, args);
	va_end(args);
	Dlg.ShowText(log_buf, bTime);
}
void product_obj::log(bool bTime, bool bReturn, const char* fmt, ...)
{
	static char log_buf[512] = { 0 };
	va_list args;
	size_t length;
	memset(log_buf, 0, sizeof(log_buf));
	va_start(args, fmt);
	length = vsnprintf(log_buf, sizeof(log_buf) - 1, fmt, args);
	va_end(args);
	Dlg.ShowText(log_buf, bTime, bReturn);
}
void product_obj::msg(const char* const str)
{
	//Dlg.m_EditShow.ReplaceSel(_T(str));
	Dlg.ShowText(_T(str), true, false);
}



