
/**
 * uart
 */

#ifndef   _UART_OBJ_H_
#define   _UART_OBJ_H_

#include <stdint.h>
#include <stdlib.h>
//#include <vector>

// 串口接口定义
class uart_obj 
{
public:
	uart_obj()
	{
		;
	}
	~uart_obj()
	{
		;
	}

//protected:
	virtual int GetConnectedTypeAndPort(BYTE& nType, BYTE& nPort) = 0;
	virtual bool ComOpen(BYTE nPort) = 0;
	virtual bool ComClose() = 0;
	//bool GetCommPort(vector<CString>& vecstrComm, vector<CString>& vecstrPort);
	//virtual bool SaveCommPort2Var(vector<CString> vecstrComm, vector<CString> vecstrPort) = 0;
	virtual int recv(void* const nBuff, const uint16_t nLen, const DWORD dwTimeout = 2 * 1000) = 0;
	virtual int send(const void* const nBuff, uint16_t nLen) = 0;
protected:
	;
private:
	;

};

#endif   // _UART_OBJ_H_
