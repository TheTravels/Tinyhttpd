/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : thread_list.c
* Author             : Merafour
* Last Modified Date : 06/15/2019
* Description        : Thread List.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "thread_list.h"
#include "agreement/agreement.h"
#include <pthread.h>
#include <assert.h>

#ifdef offsetof
#undef offsetof
#endif
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:   the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({          \
    const typeof(((type *)0)->member)*__mptr = (ptr);    \
             (type *)((char *)__mptr - offsetof(type, member)); })

static struct device_list device_buffer[2048];
static struct list_head head_online;  // 在线设备列表
static struct list_head head_free;    // 空闲链表
static pthread_mutex_t list_lock;     // 锁

void thread_list_init(void)
{
    int _size = sizeof (device_buffer)/sizeof(device_buffer[0]);
    int i = 0;
    static struct list_head *_list;
    pthread_mutex_init (&list_lock, NULL);
    INIT_LIST_HEAD(&head_online);
    INIT_LIST_HEAD(&head_free);
    //printf("head_free: 0x%08X 0x%08X 0x%08X \n", _free, _free->next, _free->prev); fflush(stdout);
    pthread_mutex_lock (&list_lock);
    memset(device_buffer, 0, sizeof (device_buffer));
    for(i=0; i<_size; i++)
    {
        _list = &device_buffer[i].list;
        list_add(_list, &head_free);
    }
    pthread_mutex_unlock (&list_lock);
}

struct device_list* get_thread_free(void)
{
    struct device_list* dev;
    struct list_head *node=NULL;
    //struct device_data* data;
    pthread_mutex_lock (&list_lock);
    if(list_empty(&head_free))
    {
        pthread_mutex_unlock (&list_lock);
        return NULL;
    }
//    data = (struct device_data*)malloc(sizeof (struct device_data));
//    if(NULL==data) return  NULL;
//    memset(data, 0, sizeof (struct device_data));
//    INIT_LIST_HEAD(&data->list);
    //printf("get_thread_free data: 0x%08X\n", data); fflush(stdout);
    //for_each(&data->list);

    node = head_free.next;
    list_del(node);
    dev = container_of(node, struct device_list, list);
    //list_add(&(data->list), &(dev->msg_list));

    //printf("get_thread_free dev: 0x%08X\n", dev); fflush(stdout);
    //for_each(&data->list);
    pthread_mutex_unlock (&list_lock);
    memset(dev, 0, sizeof (struct device_list));
    dev->fw_update = 0;
    return dev;
}

void online_thread_add(struct device_list* dev)
{
    pthread_mutex_lock (&list_lock);
    list_add(&(dev->list), &head_online);
    pthread_mutex_unlock (&list_lock);
    //printf("online_thread_add dev: 0x%08X\n", dev); fflush(stdout);
}
void online_thread_for_each(void* client, void (*call)(void* client, const struct device_list* const device, const uint8_t pack[], const uint16_t _psize))
{
    struct list_head *head=NULL;
//    struct list_head *pos=NULL;
//    struct list_head *n=NULL;
    struct list_head *node=NULL;
    struct device_list* dev;
    //struct device_data* data;
    pthread_mutex_lock (&list_lock);
    head= &head_online;
    //uint16_t index=0;

    list_for_each(node, head)
    {
        dev = container_of(node, struct device_list, list);
        if(NULL!=call) call(client, dev, dev->cache, dev->len); //
//        list_for_each_safe(pos, n, &(dev->msg_list))
//        {
//            data = (struct device_data*)container_of(pos, struct device_data, list);
//            index = data->read;
//            if(index==data->write) continue;
//            call(client, dev, data->item[index].data, data->item[index].len); //
//            // update read
//            index++;
//            if(index>=DEVICE_ITEM_SIZE) index = 0;
//            data->read = index;
//        }
    }
    pthread_mutex_unlock (&list_lock);
}
void online_thread_free(struct device_list* dev)
{
//    struct list_head *head=NULL;
//    struct list_head *pos=NULL;
//    struct list_head *n=NULL;
    //struct device_data* data;
    pthread_mutex_lock (&list_lock);
    list_del(&(dev->list));
    list_add(&(dev->list), &head_free);
    pthread_mutex_unlock (&list_lock);
    //head = &(dev->msg_list);
    //printf("dev: 0x%08X\n", dev); fflush(stdout);
    //for_each(head);

    /*
     * list_for_each 仅适合遍历索引链表
     * list_for_each_safe 可用于删除链表中的数据，释放内存
     */
//    list_for_each_safe(pos, n, head)
//    {
//        data = (struct device_data*)container_of(pos, struct device_data, list);
//        //printf("data: 0x%08X\n", data); fflush(stdout);
//        list_del(pos);
//        free(data);
//    }
}








