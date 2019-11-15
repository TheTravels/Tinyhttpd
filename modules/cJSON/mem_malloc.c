/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : mem_malloc.c
* Author             : Merafour
* Last Modified Date : 01/10/2019
* Description        : dynamic memory allocation library.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#include "mem_malloc.h"	
#include <string.h>

#ifndef MEM_BLOCK_WIDTH
#define MEM_BLOCK_WIDTH			4        /* 内存块宽度 */
#endif
#ifndef MEM_MAX_SIZE
#define MEM_MAX_SIZE			2*1024   /* 最大管理内存 2K */
#endif

#define MEM_BLOCK_SIZE			(0x1<<MEM_BLOCK_WIDTH)                  /* 内存块大小(2^MEM_BLOCK_WIDTH), 单位字节 */
/*#define MEM_ALLOC_TABLE_SIZE	MEM_MAX_SIZE/MEM_BLOCK_SIZE 	         内存表大小 */
#define MEM_ALLOC_TABLE_SIZE	(((MEM_MAX_SIZE)>>MEM_BLOCK_WIDTH)+1) 	/* 内存表大小 */

/* mem align 4 byte */
/*__align(4) static uint8_t mem_ram[MEM_ALLOC_TABLE_SIZE<<MEM_BLOCK_WIDTH]={0};			   */ /* SRAM内存池 */
static uint8_t __attribute__ ((aligned (16))) mem_ram[MEM_ALLOC_TABLE_SIZE<<MEM_BLOCK_WIDTH]={0};			    /* SRAM内存池 */
/* SRAM内存表 */
#if MAX_BLOCK_SIZE == MAX_BLOCK_SIZE_8bit
static uint8_t mem_map[MEM_ALLOC_TABLE_SIZE]={0};			        
#elif MAX_BLOCK_SIZE == MAX_BLOCK_SIZE_16bit
static uint16_t mem_map[MEM_ALLOC_TABLE_SIZE]={0};			        
#elif MAX_BLOCK_SIZE == MAX_BLOCK_SIZE_32bit
static uint32_t mem_map[MEM_ALLOC_TABLE_SIZE]={0};			        
#else
#error "Don't set MAX_BLOCK_SIZE!"
#endif
/* 内存管理参数 */
static const uint32_t mem_map_size=MEM_ALLOC_TABLE_SIZE;		    /* 内存表大小 */
static uint8_t ready = 0;

/* 内存管理初始化 */
static void init(void)  
{  
    memset(mem_map, 0,sizeof(mem_map)); 
	memset(mem_ram, 0, sizeof(mem_ram));
	ready = 1;
}
/**  
 * 获取内存使用率
 * 返回值:使用率(0~100)
*/
uint8_t mem_perused(void)  
{  
    uint32_t used=0;  
    uint32_t i;  
    for(i=0;i<mem_map_size;i++)  
    {  
		if(mem_map[i])used+=100; 
    } 
    return (uint8_t)((used)/(mem_map_size));  
}  
void* mem_malloc(const uint32_t size)
{  
    uint32_t offset=0;  
    uint32_t block_size;
	uint32_t block_addr;
	uint16_t block_cnt=0;
    uint32_t i;  
	if(!ready) init();
    if(size==0) return NULL;
	
	block_size=size>>MEM_BLOCK_WIDTH;
	// check block_size
#if MAX_BLOCK_SIZE == MAX_BLOCK_SIZE_8bit
	if(block_size&0xFFFFFF00) return NULL;
#elif MAX_BLOCK_SIZE == MAX_BLOCK_SIZE_16bit
	if(block_size&0xFFFF0000) return NULL;
#endif
    if(size&((0x1<<MEM_BLOCK_WIDTH)-1)) block_size++; 
    for(offset=mem_map_size-1; ; offset--) // search empty mem
    {
		if(!mem_map[offset])block_cnt++;   // is empty
		else block_cnt=0; 
		if(block_cnt==block_size)	       // search ok
		{
            for(i=0;i<block_size;i++)      // mark using
            {  
				//mem_map[offset+i]=block_size;
#if MAX_BLOCK_SIZE == MAX_BLOCK_SIZE_8bit
                mem_map[offset+i]=(uint8_t)block_size;		        
#elif MAX_BLOCK_SIZE == MAX_BLOCK_SIZE_16bit
                mem_map[offset+i]=(uint16_t)block_size;		        
#elif MAX_BLOCK_SIZE == MAX_BLOCK_SIZE_32bit
                mem_map[offset+i]=block_size;			        
#else
#error "Don't set MAX_BLOCK_SIZE!"
#endif
            }
			block_addr = offset<<MEM_BLOCK_WIDTH;
			return &mem_ram[block_addr];
		}
		if(0==offset) // don't have empty mem
		{
			break;
		}
    }  
    return NULL;
}
static inline void _free(const uint32_t block_offset)  
{  
    uint32_t i;  
	uint32_t block_size=mem_map[block_offset];
	for(i=0;i<block_size;i++)
	{
		// mark empty mem
		mem_map[block_offset+i]=0;  
	}  
}
void mem_free(void *ptr)  
{  
	uint32_t offset;  
    if(ptr==NULL) return ;
 	offset=(uint32_t)((uint8_t*)ptr-(uint8_t*)mem_ram);
	if(!ready)
	{   
		init();
        return ;
    }
	if(offset<sizeof(mem_ram))
    {
		// free, block addr: offset>>MEM_BLOCK_WIDTH
		_free(offset>>MEM_BLOCK_WIDTH);
	}    
}  

void* mem_realloc(void *ptr, const uint32_t size)  
{
	void* _mem=NULL;
	_mem = mem_malloc(size); 
    if(NULL==_mem) 
	{
		return NULL;
	}
    else  
    {
	    memcpy(_mem,ptr,size);	// copy data  
        mem_free(ptr);  		// free old mem
        return _mem;  			// new mem
    } 	
}

void* smem_malloc(size_t size)				            /* 内存分配 */
{
	return mem_malloc((const uint32_t)size);
}
void* smem_realloc(void *ptr, size_t size)	            /* 重新分配内存 */
{
	return mem_realloc(ptr, (const uint32_t)size);
}

