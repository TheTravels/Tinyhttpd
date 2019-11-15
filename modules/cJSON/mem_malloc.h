/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : mem_malloc.h
* Author             : Merafour
* Last Modified Date : 01/10/2019
* Description        : dynamic memory allocation library.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#ifndef _MEM_MALLOC_H_
#define _MEM_MALLOC_H_

#ifdef __cplusplus
extern "C"
{
#endif
	
#include <stdint.h>
#include <stddef.h>
#ifndef NULL
#define NULL 0
#endif

/**
 * 单次分配最大内存数量
 * 8bit: 2^8-1=255; 16bit: 2^16-1=65535; 32bit: 2^32-1, MCU没有这么大内存,不用该设置
 * 作用: 8bit 相比 16bit 内存表可节省一半,在内存分配块数量不超过 255时可节省内存,这对于单片机很有用
 * 内存块数量计算: size >> MEM_BLOCK_WIDTH
 * 同时需要注意 MEM_BLOCK_WIDTH 越大即单个内存块越大(2^MEM_BLOCK_WIDTH), 在动态内存分配过程中浪费越多,
 * 因为不可能每次都分配的刚刚好, 所以在单片机中为了减少内存浪费请根据需要的最大与最小内存设置 MAX_BLOCK_SIZE 与 MEM_BLOCK_WIDTH.
 * 同时可以通过 mem_perused 函数观察内存使用率以设置最优值.
 */
#define MAX_BLOCK_SIZE_8bit     0x01
#define MAX_BLOCK_SIZE_16bit    0x02
#define MAX_BLOCK_SIZE_32bit    0x04
#define MAX_BLOCK_SIZE   MAX_BLOCK_SIZE_16bit

/**
 * 内存块宽度, 内存块大小(2^MEM_BLOCK_WIDTH), 单位字节.用于加快内存分配时的运算速度
 */
#define MEM_BLOCK_WIDTH			4                               
/**
 * 最大管理内存 10K
 */
#ifndef MEM_MAX_SIZE
#define MEM_MAX_SIZE			(1000*1024)
#endif

extern uint8_t mem_perused(void);					            /* 内存使用率 */
extern void  mem_free(void *ptr);  				                /* 内存释放 */
extern void* mem_malloc(const uint32_t size);				    /* 内存分配 */
extern void* mem_realloc(void *ptr, const uint32_t size);	    /* 重新分配内存 */
/**    size_t    **/
extern void* smem_malloc(size_t size);				            /* 内存分配 */
extern void* smem_realloc(void *ptr, size_t size);	            /* 重新分配内存 */

#ifdef __cplusplus
}
#endif

#endif /* _MEM_MALLOC_H_ */

