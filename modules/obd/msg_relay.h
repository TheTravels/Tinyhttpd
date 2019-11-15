/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : json_list.h
* Author             : Merafour
* Last Modified Date : 06/15/2019
* Description        : JSON List.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#ifndef _MSG_RELAY_H_
#define _MSG_RELAY_H_

#ifdef __cplusplus
extern "C"
{
#endif
	
#include <stdint.h>
#include <stddef.h>


extern int relay_init(const char host[], const int port);
extern int relay_exit(const int socket);
extern int relay_msg(const int socket, const void* data, const uint16_t _dsize);



#ifdef __cplusplus
}
#endif

#endif /* _MSG_RELAY_H_ */

