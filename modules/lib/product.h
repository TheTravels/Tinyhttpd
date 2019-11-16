
/**
 * product
 */

#ifndef   _PRODUCT_H_
#define   _PRODUCT_H_

//#include "VCI_III_SIDDlg.h"
#include <stdint.h>
#include <stdlib.h>
//#include <vector>
//#include "orders_obd.h"
//#include "uart_obj.h"

#define SN_NUMBER    0x0004         // 数字码
#define SN_GENERAL   0x0005         // 普码
#define SN_SIMPLE    0x0006         // 简码
#define SN_FEATURE   0x0007         // 特征码feature

enum Instruction_Sets {
	ORDER_VCI = 0x0001,
	ORDER_OBD = 0x0002,          // 旧码
	ORDER_OBD_DEVELOP = 0x0003,  // 开发模式
	//SN_NUMBER  = 0x0004,         // 数字码
	//SN_GENERAL = 0x0005,         // 普码
	//SN_SIMPLE  = 0x0006,         // 简码
	//SN_FEATURE = 0x0007,         // 特征码feature
	ORDER_OBD_GENERAL = 0x0008,  // 普码
	ORDER_OBD_SIMPLE  = 0x0009,  // 简码
	ORDER_OBD_FEATURE = 0x000A,  // 特征码feature
	ORDER_NULL,
};
struct order_func {
	const enum Instruction_Sets cmd;
	const char* item;
};
struct product_type {
	const enum Instruction_Sets cmd;
	uint32_t imodel;
};
#if 1
 /**
  *  |yearh|batch|yearl|month|model|number|fixed|
  *  |  1  |  2  |  1  |  2  |  2  |  3   |  1  |
  *  |                   12B                    |
  */
struct serial_number {
	uint8_t yearh;      // 年
	uint8_t yearl;      // 年
	uint8_t batch[2];   // 批次
	uint8_t month[2];   // 月份
	uint8_t model[2];   // 型号
	uint8_t number[3];  // 序号/数量
	uint8_t fixed;      // 固定值
	// 整数
	uint8_t iyear;
	uint8_t ibatch;
	uint8_t imonth;
	uint8_t imodel;
	uint16_t inumber;
};
#endif

/**
 *  |区分码 |型号1 |型号2 |地区|年份 |月份 |版本 |校验码 |批号 |序号|
 *  |   1  |  1  |  1  |  1 |  1 |  1 |  1 |   1  |  1 |  3 |
 *  |                        12B                            |
 */
// 序列号通用编码
//#define SN_GENERAL   0x04  // 普码
//#define SN_SIMPLE    0x05  // 简码
//#define SN_FEATURE   0x06  // 特征码feature
struct serial_number_general {
	uint8_t type;       // 普码、简码、特殊编码标志
    char code;          // 区分码
    uint8_t model1;     // 型号1
    uint8_t model2;     // 型号2
	uint8_t area;       // 地区
	uint8_t year;       // 年份
	uint8_t month;      // 月份
	uint8_t ver;        // 版本
	uint8_t crc;        // 校验码
	uint8_t batch;      // 批号
	uint8_t number[3];  // 序号/数量
    // 数字码数据
    uint8_t fixed;      // 固定值
	// 整数
	uint16_t iyear;
	uint8_t ibatch;
	uint8_t imonth;
	uint32_t imodel;
	uint32_t inumber;
};
#define IMODEL(C,M1,M2) (((C&0xFF) << 16) | ((M1&0xFF) << 8) | ((M2&0xFF) << 0))

extern const uint32_t imodel_OBD_4G;
extern const uint32_t imodel_OBD_4G_feature;
#if 0
class CVCIIIISIDDlg;

class product_obj 
{
public:
	product_obj(CVCIIIISIDDlg &_pDlg, uart_obj* const _uart, const char* _obj) : Dlg(_pDlg), uart(_uart), obj(_obj)
	{
		m_handle=NULL;
	}

	virtual int SN_Write(CString strPid, const int _quality, CString _host) = 0;
	// 编程设备
	/*
	功能：编程数据到设备
	参数：
			__sn:序列号，数字码/普码/简码/特征码
			_host:服务器 IP/域名:端口
			_quality:质检选项
			_develop:开发/测试选项
			_encryp:软件加密选项
			_key:加密密钥
	*/
	int program_device(const char __sn[], const char _host[], const int _quality, const int _develop, const int _encryp, const int _key[]); 
	virtual int pack_check(const char _data[], const int _dsize) = 0;
	virtual int analyse_serial_number(struct serial_number* _sn, CString strPid);
	virtual int analyse_serial_number_general(struct serial_number_general* _sn, CString strPid);
	virtual int get_serial_number_general(char* const _sn, CString strPid);
	virtual int check_serial_number(CString strPid, const uint32_t _imodel) = 0;

	static int cstring_to_char(CString str, char buf[], const int _bsize)
	{
		int i;
		int len = str.GetLength();
		//CString log;
		if (len >= _bsize) len = _bsize - 1;
		memset(buf, 0, _bsize);
		for (i = 0; i < len; i++)
		{
			buf[i] = (char)str.GetAt(i);
		}
		return len;
	}

	const char* const obj;

protected:
	//bool ComOpen(BYTE nPort);
	//bool ComClose();

	/*int SN_Write_VCI(CVCIIIISIDDlg* pDlg, CString strPid);
	int SN_Write_OBD(CVCIIIISIDDlg* pDlg, CString strPid, const int _quality);
	int SN_Write_OBD_Send(CVCIIIISIDDlg* pDlg, CString strText, orders_obd& order, const uint8_t _order, uint8_t _data[], const uint16_t _dsize, uint8_t _buf[], const uint16_t _bsize);
	*/
	int program_orders(const uint8_t cmd_list[], const uint8_t cmd_list_size, const char _sn[], const char _host[], const char _att[], const int _encryp, const int _key[], const uint8_t _sql_cmd);
	CString ByteArray2CString(BYTE* nBuf, BYTE nLen)
	{
		int i;
		CString str = _T(""), tmp;
		for (i = 0; i < nLen; i++)
		{
			tmp.Format(_T("%c"), nBuf[i]);
			str += tmp;
		}
		return str;
	}
	void msg(const char* const str);
	/*{
		//Dlg.m_EditShow.ReplaceSel(_T(str));
		Dlg.ShowText(_T(str), true, false);
	}*/
	/*void exit(void)
	{
		Dlg.PidThread = NULL;
		Dlg.ComClose();
	}*/
	void ShowText(CString strText, bool bTime = true, bool bReturn = true);
	void Log(bool bTime, const char* fmt, ...);
	void log(bool bTime, bool bReturn, const char* fmt, ...);
	bool Send(const void* const nBuff, const uint16_t nLen);
	bool Recv(void* const nBuff, const uint16_t nLen, DWORD dwTimeout = 2 * 1000);
	/*
	|   序列号   -       域名      :端口|
	"102905420108-obd4g.zodtool.com:9910"
	*/
	//int __read_host(const char domain[64], char host[32])
	int analysis_host(const char domain[], char host[])
	{
		char buf[64];//="102905420108-obd4g.zodtool.com:9910";
		int index = 0;
		int len = 0;
		int _host_pos;
		int _port_pos;
		int port;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, domain, sizeof(buf));
		//for (index = 0; index < sizeof(buf); index++) if ('-' == buf[index]) break;
		//if (index >= sizeof(buf)) return -1;
		buf[sizeof(buf) - 1] = 0;
		_host_pos = 0;
		for (index = _host_pos; index < sizeof(buf); index++) if (':' == buf[index]) break;
		if (index >= sizeof(buf)) return -2;
		_port_pos = index + 1;
		port = atoi(&buf[_port_pos]);
		len = _port_pos - _host_pos - 1;
		if (len <= 0) return -3;
		memcpy(host, &buf[_host_pos], len);
		host[len] = '\0';
		return port;
	}

	HANDLE m_handle;
	CVCIIIISIDDlg & Dlg;
	uart_obj* const uart;
private:
	int __analyse_serial_number(struct serial_number* const _sn, const char* const data, const int _dsize);
	int __analyse_serial_number_general(struct serial_number_general* const _sn, const char* const data, const int _dsize);
	int __get_serial_number_general(char* const _sn, const char* const data, const int _dsize);

};

class product_vci : public product_obj
{
public:
	product_vci(CVCIIIISIDDlg& _pDlg, uart_obj* const _uart) : product_obj(_pDlg, _uart, "product_vci")
	{
		;
	}

	virtual int SN_Write(CString strPid, const int _quality, CString _host) override;
	virtual int pack_check(const char _data[], const int _dsize) override
	{
		return 0;
	}
	virtual int check_serial_number(CString strPid, const uint32_t _imodel) override
	{
		struct serial_number sn;
		if (0 != analyse_serial_number(&sn, strPid)) return -1;
		if (('T' == sn.fixed) && ('9' == sn.fixed))
		{
			return 0;
		}
		return -2;
	}

protected:

private:
	;

};
class product_obd4g : public product_obj
{
public:
	product_obd4g(CVCIIIISIDDlg& _pDlg, uart_obj* const _uart) : product_obj(_pDlg, _uart, "product_obd4g")
	{
		;
	}

	virtual int SN_Write(CString strPid, const int _quality, CString _host) override;
	virtual int pack_check(const char _data[], const int _dsize) override;
	virtual int check_serial_number(CString strPid, const uint32_t _imodel) override
	{
		struct serial_number sn;
		if (0 != analyse_serial_number(&sn, strPid)) return -1;
		//if ((42 == sn.imodel) && (('8' == sn.fixed) || ('T' == sn.fixed)))
		if ((_imodel == sn.imodel) && (('8' == sn.fixed) || ('T' == sn.fixed)))
		{
			return 0;
		}
		return -2;
	}

protected:
	virtual int SN_Write_OBD(CString strPid, const int _quality, const char _host[]);
	int SN_Write_OBD_bak(CString strPid, const int _quality, const char _host[]);
	int write_orders(const uint8_t cmd_list[], const uint8_t cmd_list_size, const char _sn[], const char _host[], const uint8_t _sql_cmd);
private:
	//int analysis_host(const char domain[], char host[]);

};
class product_obd4g_develop : public product_obd4g
{
public:
	product_obd4g_develop(CVCIIIISIDDlg& _pDlg, uart_obj* const _uart) : product_obd4g(_pDlg, _uart)
	{
		;
	}

protected:
	virtual int SN_Write_OBD(CString strPid, const int _quality, const char _host[]) override;
	int SN_Write_OBD_bak(CString strPid, const int _quality, const char _host[]);
private:
	//int analysis_host(const char domain[], char host[]);

};
class product_sn_general : public product_obj
{
public:
	product_sn_general(CVCIIIISIDDlg& _pDlg, uart_obj* const _uart) : product_obj(_pDlg, _uart, "product_sn_general")
	{
		;
	}

	virtual int SN_Write(CString strPid, const int _quality, CString _host) override;
	virtual int pack_check(const char _data[], const int _dsize) override;
	virtual int check_serial_number(CString strPid, const uint32_t _imodel) override
	{
		struct serial_number_general sn;
		if (0 != analyse_serial_number_general(&sn, strPid)) return -1;
		//if (imodel_OBD_4G_feature == sn.imodel)
		if (_imodel == sn.imodel)
		{
			return 0;
		}
		return -2;
	}

protected:
	virtual int SN_Write_OBD(CString strPid, const int _quality, const char _host[]);
	int write_orders(const uint8_t cmd_list[], const uint8_t cmd_list_size, const char _sn[], const char _host[], const char _att[], const uint8_t _sql_cmd);
private:
	//int analysis_host(const char domain[], char host[]);

};

#endif
#endif   // _PRODUCT_H_
