#ifndef _ACCEPT_REQUEST_H_
#define _ACCEPT_REQUEST_H_

#ifdef __cplusplus
extern "C"
{
#endif
	
#include <stdint.h>
#include <stddef.h>


/*struct device_list{   		 // 设备列表
    struct socket_list* next;    // 下一个设备
    int socket;       		 // TCP / IP
//    struct report_save* data;    // 数据
};*/

extern void accept_request(void *);
extern void error_die(const char *);

#ifdef __cplusplus
}
#endif

#endif /* _ACCEPT_REQUEST_H_ */

