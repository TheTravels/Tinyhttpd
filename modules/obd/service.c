/* J. David's webserver */
/* This is a simple webserver.
 * Created November 1999 by J. David Blackstone.
 * CSE 4344 (Network concepts), Prof. Zeigler
 * University of Texas at Arlington
 */
/* This program compiles for Sparc Solaris 2.6.
 * To compile for Linux:
 *  1) Comment out the #include <pthread.h> line.
 *  2) Comment out the line that defines the variable newthread.
 *  3) Comment out the two lines that run pthread_create().
 *  4) Uncomment the line that runs accept_request().
 *  5) Remove -lsocket from the Makefile.
 */
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <time.h>
#include <getopt.h>
//#include "accept_request.h"

#include "../agreement/agreement.h"
#include "json_list.h"
#include "thread_list.h"
#include "trunking.h"
#include <pthread.h>
#include "../agreement/obd_agree_fops.h"
#include "../agreement/obd_agree_shanghai.h"
#include "../agreement/obd_agree_yunjing.h"

#ifndef GCC_BUILD
#define GCC_BUILD   0
#endif

#if GCC_BUILD
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/wait.h>
#include "lock.h"
int startup(u_short *);
#else
int pthread_lock(void){ return 0;}
int pthread_unlock(void){ return 0;}
#endif

#define ISspace(x) isspace((int)(x))

#define SERVER_STRING "Server: jdbhttpd/0.1.0\r\n"
#define STDIN   0
#define STDOUT  1
#define STDERR  2

void bad_request(int);
void cat(int, FILE *);
void cannot_execute(int);
void error_die(const char *);
void execute_cgi(int, const char *, const char *, const char *);
int get_line(int, char *, int);
void headers(int, const char *);
void not_found(int);
void serve_file(int, const char *);
void unimplemented(int);
int read_threads(int sock, char *buf, int size, int *status);

static int msg_print(char *__stream, const size_t __n, const char *__format, ...)
{
    char* text=NULL;
    size_t _size=0;
    va_list ap;
    _size = strlen(__stream);
    if(_size>=__n) return -1;
    text = &__stream[_size];
    va_start(ap, __format);
    //vprintf(__format, ap);
    //snprintf(text, sizeof (text), __format, ap);
    vsprintf(text, __format, ap);
    va_end(ap);
    return 0;
}

/**********************************************************************/
/* A request has caused a call to accept() on the server port to
 * return.  Process the request appropriately.
 * Parameters: the socket connected to the client */
/**********************************************************************/
int save_log = 0;
#include "DateTime.h"
#include <stdio.h>
void UTC2file(const uint32_t times, void* const buf, const size_t _size)
{
    DateTime      utctime   = {.year = 1970, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 0};
    DateTime      localtime = {.year = 1970, .month = 1, .day = 1, .hour = 8, .minute = 0, .second = 0};
    if(times > INT32_MAX)
    {
        utctime = GregorianCalendarDateAddSecond(utctime, INT32_MAX);
        utctime = GregorianCalendarDateAddSecond(utctime, (int)(times - INT32_MAX));
    }
    else
    {
        utctime = GregorianCalendarDateAddSecond(utctime, (int)times);
    }

    GregorianCalendarDateToModifiedJulianDate(utctime);
    localtime = GregorianCalendarDateAddHour(utctime, 8);
    snprintf((char *)buf, (size_t)_size, "log/log-%d-%.2d-%.2d-%02d%02d%02d.txt", localtime.year, localtime.month, localtime.day, localtime.hour, localtime.minute, localtime.second);
}
void UTC2file_bin(const uint32_t times, void* const buf, const size_t _size)
{
    DateTime      utctime   = {.year = 1970, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 0};
    DateTime      localtime = {.year = 1970, .month = 1, .day = 1, .hour = 8, .minute = 0, .second = 0};
    if(times > INT32_MAX)
    {
        utctime = GregorianCalendarDateAddSecond(utctime, INT32_MAX);
        utctime = GregorianCalendarDateAddSecond(utctime, (int)(times - INT32_MAX));
    }
    else
    {
        utctime = GregorianCalendarDateAddSecond(utctime, (int)times);
    }

    GregorianCalendarDateToModifiedJulianDate(utctime);
    localtime = GregorianCalendarDateAddHour(utctime, 8);
    snprintf((char *)buf, (size_t)_size, "log/log-%d-%.2d-%.2d-%02d%02d%02d.bin", localtime.year, localtime.month, localtime.day, localtime.hour, localtime.minute, localtime.second);
}

/*static*/ void csend(const int sockfd, const void *buf, const uint16_t len)
{
#if GCC_BUILD
    send(sockfd, buf, len, 0);
#else
    (void)sockfd;
    (void)buf;
    (void)len;
#endif
}

#ifdef offsetof
#undef offsetof
#endif
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
/**
 *  * container_of - cast a member of a structure out to the containing structure
 *   * @ptr:    the pointer to the member.
 *    * @type:   the type of the container struct this is embedded in.
 *     * @member: the name of the member within the struct.
 *      *
 *       */
#define container_of(ptr, type, member) ({          \
    const typeof(((type *)0)->member)*__mptr = (ptr);    \
    (type *)((char *)__mptr - offsetof(type, member)); })

#if 0
const char relay_host[] = "39.108.72.130";
const int relay_port = 9910;
#else
const char relay_host[] = "106.14.52.96";
const int relay_port = 6097;
#endif

static struct list_head list_task; // 任务链表
static pthread_mutex_t list_task_mutex;
static int list_task_init = 0;     // 初始化标志
static int thread_trunk_max = 0;
static struct list_head list_trunk; // 中继任务链表

static pthread_mutex_t obd_mutex;
static pthread_mutex_t trunk_mutex;
static int request_count = 0;

// 获取新请求
struct device_list* new_request(struct list_head* tasks_head)
{
    struct device_list* device=NULL;
    struct list_head* node;
    // 切换请求, move node
    node = tasks_head->prev;
    list_del(node);
    list_add(node, tasks_head);
    while(node==(tasks_head)) node = node->next;
    device = container_of(node, struct device_list, list_work);
    return device;
}
// 获取新任务
int new_task(int tasks_max, struct list_head* _head, struct list_head* tasks, pthread_mutex_t* mutex)
{
    if((tasks_max<32) && (0==pthread_mutex_trylock(mutex)))
    {
        if(!list_empty(_head))  // 获取新的任务
        {
            struct list_head* new_task=NULL;
            new_task = _head->next; // 取出任务
            list_del(new_task);
            //            device = container_of(new_task, struct device_list, list_work);
            //            list_add(&(device->list_work), &tasks);
            list_add(new_task, tasks);
            tasks_max++;
        }
        pthread_mutex_unlock(mutex);
    }
    return tasks_max;
}

void thread_trunking(void *arg)
{
    struct device_list* device=NULL;
    struct list_head tasks; // 任务链表
    int tasks_max;          // 任务数
    //struct list_head* node;
    //(void)arg;
    tasks_max=0;
    device = (struct device_list*)arg;
    INIT_LIST_HEAD(&tasks);
    list_add(&(device->list_work), &tasks);
    tasks_max++;
    //node = tasks.next;
    //device = container_of(node, struct device_list, list_work);
    device = new_request(&tasks);
    printf("中继线程启动 TCP: %d\n\n", device->socket);
    while(1)
    {
        if(0!=trunking(device->socket)) // close
        {
            // 连接已断开,删除该任务
            //break;
            printf("\n连接断开[%d]\n", device->socket);
            //node = node->next;
            if(device->relay_fd>=0) relay_exit(device->relay_fd);
            close(device->socket);
            if(NULL!=device) online_thread_free(device);
            list_del(&(device->list_work));
        }
        // 一个线程处理多个请求
#if 0
        if((tasks_max<32) && (0==pthread_mutex_trylock(&trunk_mutex)))
        {
            if(!list_empty(&list_trunk))  // 获取新的任务
            {
                struct list_head* new_task=NULL;
                new_task = list_trunk.next; // 取出任务
                list_del(new_task);
                device = container_of(new_task, struct device_list, list_work);
                list_add(&(device->list_work), &tasks);
                tasks_max++;
            }
            pthread_mutex_unlock(&trunk_mutex);
        }
#endif
        tasks_max = new_task(tasks_max, &list_trunk, &tasks, &trunk_mutex);
        if(list_empty(&tasks))  // 无需要处理的任务
        {
            break;
        }
        //        // 切换请求, move node
        //        node = tasks.prev;
        //        list_del(node);
        //        list_add(node, &tasks);
        //        while(node==(&tasks)) node = node->next;
        //        device = container_of(node, struct device_list, list_work);
        device = new_request(&tasks);
    }
}
extern int relay;
int relay = 0;

static void save_to_file(const time_t timer, const char buf[], size_t numchars)
{
    FILE* fd = NULL;
    char filename[255];
    size_t i;
    fd = fopen(filename, "w+");
    if(NULL!=fd)
    {
#if 0
        fwrite(buf, numchars, 1, fd);
#else
        for(i=0; i<numchars; i++)
        {
            fprintf(fd, " %02X", buf[i]&0xFF);
        }
        //fprintf(fd, "\n");
#endif
        fflush(fd);
        fclose(fd);
    }
    memset(filename, 0, sizeof(filename));
    UTC2file_bin(timer, filename, sizeof(filename));
    fd = NULL;
    fd = fopen(filename, "w+");
    if(NULL!=fd)
    {
        fwrite(buf, numchars, 1, fd);
        fflush(fd);
        fclose(fd);
    }
}
#if 1
static char server_log_path[128] = "./log/dame.txt";
static int __server_log_write_to_file(char *__stream, const size_t __n, const char *_path)
{
    FILE* fd=NULL;
    //pthread_mutex_lock (&vin_lock);
    fd = fopen(_path, "ab");
    if(NULL!=fd)
    {
        fwrite(__stream, __n, 1, fd);
        fflush(fd);
        fclose(fd);
    }
    //printf("%s", __stream); fflush(stdout);
    //pthread_mutex_unlock (&vin_lock);
    return 0;
}
int server_log_write_to_file(char *__stream, const size_t __n)
{
    return __server_log_write_to_file(__stream, __n, server_log_path);
}
#endif
void thread_request(void *arg)
{
    const char viewer[] = "SocketViewer";
    //const struct agreement_ofp* _agree_obd=NULL;
    //const struct agreement_ofp* _agree_obd_yj=NULL;
    uint8_t msg_buf[4096];
    //int client = 0;//(intptr_t)arg;
    //char buf[1024];
    int timeout=0;
    size_t numchars;
    //size_t numchars2;
    char _print_buf[1024*20];
    const unsigned int _print_bsize = sizeof(_print_buf);
    char method[255];
    char filename[255];
    clock_t start, finish;
    double  duration;
    int recv_status=1;
    time_t time2, time1;
    //    FILE* fd;
    char url[255];
    char path[512];
    size_t i, j;
    time_t timer;
    struct stat st;
    struct device_list* device=NULL;
    //struct device_data  *msg_cache;
    int cgi = 0;      /* becomes true if server decides this is a CGI
               * program */
    char *query_string = NULL;
    struct list_head tasks; // 任务链表
    int tasks_max;          // 任务数
    //struct list_head* node;
    int print=0;
    char _obd_obj_buf[sizeof(struct obd_agree_obj)];
    struct obd_agree_obj* const _obd_obj = obd_agree_obj_shanghai.fops->constructed(&obd_agree_obj_shanghai, _obd_obj_buf);
    char _obd_obj_buf_yj[sizeof(struct obd_agree_obj)];
    struct obd_agree_obj* const _obd_obj_yj = obd_agree_obj_yunjing.fops->constructed(&obd_agree_obj_yunjing, _obd_obj_buf_yj);
    char __stream[1024*30] = "\0";
    struct msg_print_obj _print = {
        .fops = &_msg_print_fops,
        .__stream = __stream,
        .__n = sizeof(__stream),
    };
    struct obd_agree_ofp_data _ofp_data;

    tasks_max = 0;
    device = (struct device_list*)arg;
    INIT_LIST_HEAD(&tasks);
    list_add(&(device->list_work), &tasks);
    tasks_max++;

    //int flags = fcntl(client, F_GETFL, 0);        //获取文件的flags值。
    //fcntl(client, F_SETFL, flags | O_NONBLOCK);   //设置成非阻塞模式；
    //usleep(1000*100);   // 100ms
    //device->relay_fd = -1;

    start = clock();
    time(&time1);
    //_agree_obd = create_agree_obd_shanghai();
    _obd_obj->fops->init(_obd_obj, 0, (const uint8_t*)"IMEI1234567890ABCDEF", (const uint8_t*)"VIN0123456789ABCDEF", 2, "INFO");
    //_agree_obd_yj = create_agree_obd_yunjing();
    _obd_obj_yj->fops->init(_obd_obj_yj, 0, (const uint8_t*)"IMEI1234567890ABCDEF", (const uint8_t*)"VIN0123456789ABCDEF", 2, "INFO");
    //numchars2 = 0;
    numchars = 0;
    //msg_cache->write=0;
    //msg_cache->read=0;
    //    node = tasks.next;
    //    device = container_of(node, struct device_list, list_work);
    device = new_request(&tasks);
    //client = device->socket;
    msg_print(_print_buf, _print_bsize, "开始接收数据 TCP: %d\n\n", device->socket);
    device->last_time = 0;
    timeout=0;
    while(1)
    {
        char* const buf = device->buf;
        server_log_write_to_file(_print_buf, strlen(_print_buf));
        memset(_print_buf, 0, _print_bsize);
        //numchars = get_line(client, buf, sizeof(buf));
#define numchars2 device->buf_pos
        numchars = read_threads(device->socket, &buf[numchars2], sizeof(device->buf)-numchars2, &recv_status);
        //if(0==numchars) goto next;
        if(0==numchars)
        {
            timeout++;
            if((0==numchars2) && (timeout>10))
            {
                //printf("recv timeout...1\n");
                goto next;
            }
            else if(timeout>2000) // 2s
            {
                //printf("recv timeout...2\n");
                goto next;
            }
            usleep(1000);   // 10ms
            continue;
        }
        numchars = numchars + numchars2;
        if(0==numchars) goto next;
        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        time(&time2);
        duration = difftime(time2, time1);
        //printf( "recv time: %f seconds\n\n", duration );
        //printf( "recv time: tart:%ld  finish:%ld duration:%f seconds\n\n", start, finish, duration );
        //if(print) printf( "\nrecv time: time1:%ld  time2:%ld duration:%f seconds\n", time1, time2, duration );
        //printf( "\nrecv time: time1:%ld  time2:%ld duration:%f seconds\n", time1, time2, duration );
        //printf( "\nrecv time: time1:%ld duration:%f seconds recv:%s \n", time1, duration, buf);
        start = finish;
        time1 = time2;

        i = 0; j = 0;
        while (!ISspace(buf[i]) && (i < sizeof(method) - 1))
        {
            method[i] = buf[i];
            i++;
        }
        j=i;
        method[i] = '\0';
        // 连接已经断开或者长时间没有收到数据
        if(device->last_time<1000) device->last_time = time2;
        //if((0==recv_status) || (difftime(time2, device->last_time)>60)) // close
        if(0==recv_status) // close
        {
            // 连接已断开,删除该任务
            //break;
            msg_print(_print_buf, _print_bsize, "\n连接断开[%d 0x%08X]\n", device->socket, device);
            msg_print(_print_buf, _print_bsize, "difftime[%f] [%ld %ld]\n", difftime(time2, device->last_time), time2, device->last_time);
            //node = node->next;
            if(device->relay_fd>=0) relay_exit(device->relay_fd);
            close(device->socket);
            online_thread_free(device);
            list_del(&(device->list_work));
            numchars2 = 0;
            goto next;
        }
        //if(0==memcmp(buf, viewer, sizeof(viewer)))  // 数据中继
        if(NULL!=strstr(buf, viewer))  // 数据中继
        {
            struct list_head* node=NULL;
            device->last_time = time(NULL);
            msg_print(_print_buf, _print_bsize, "view: %s\n", viewer);
            numchars2 = 0;
            //node = node->next;
            //if(device->relay_fd>=0) relay_exit(device->relay_fd);
            //device->relay_fd = -1;
            //if(NULL!=device) online_thread_free(device);
            list_del(&(device->list_work));
            //trunking(client);
            pthread_mutex_lock(&trunk_mutex);
            list_add(&(device->list_work), &list_trunk); // 添加中继任务
            pthread_mutex_unlock(&trunk_mutex);
            sleep(1);  // 等待中继线程来处理
#if 0
            device = NULL;
            pthread_mutex_lock(&trunk_mutex);
            if(!list_empty(&list_trunk)) // 启动中继线程
            {
                struct list_head* node=NULL;
                node = list_trunk.next; // 取出任务
                list_del(node);
                device = container_of(node, struct device_list, list_work);
            }
            pthread_mutex_unlock(&trunk_mutex);
            if(NULL!=device) pool_add_worker(thread_trunking, device); // 中继线程启动
            goto next;
#endif
            device = NULL;
            pthread_mutex_lock(&trunk_mutex);
            if(!list_empty(&list_trunk)) // 启动中继线程
            {
                node = list_trunk.next; // 取出任务
                list_del(node);
                device = container_of(node, struct device_list, list_work);
                pthread_mutex_unlock(&trunk_mutex);
            }
            pthread_mutex_unlock(&trunk_mutex);
            if(NULL!=device)
            {
                // 清空任务，让给其他线程处理
                pthread_mutex_lock(&list_task_mutex);
                while(!list_empty(&tasks))
                {
                    node = tasks.next; // 取出任务
                    list_del(node);
                    list_add(node, &list_task);
                }
                tasks_max=0;
                pthread_mutex_unlock(&list_task_mutex);
                msg_print(_print_buf, _print_bsize, "任务已清空\n");
                thread_trunking(device); // 中继线程替代当前线程
                return;
            }
            //new_request(&tasks); // 中继任务已被抢占
            tasks_max = new_task(tasks_max, &list_task, &tasks, &list_task_mutex);
            goto next;
        }
        else if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
        {
            char *const data = strstr(buf, "##");
            int len = 0;
            int decode; // 解码数据
            if(NULL==data)
            {
                numchars2 = numchars;
                usleep(1000*10);   // 10ms
                continue;
            }
            len = data-buf;
            if(relay && (-1==device->relay_fd))
            {
                msg_print(_print_buf, _print_bsize, "转发连接建立中 host:%s port:%d\n", relay_host, relay_port);
                for(i=0; i<1000; i++)
                {
                    int sock=0;
                    msg_print(_print_buf, _print_bsize, "转发连接第 %ld 次建立连接 ...\n", i);
                    sock = relay_init(relay_host, relay_port);
                    device->relay_fd = sock;
                    if(sock>=0) break;
                    usleep(1000*100);   // 100ms delay
                }
                msg_print(_print_buf, _print_bsize, "转发连接建立结束 host:%s port:%d fd:%d\n\n", relay_host, relay_port, device->relay_fd);
            }
#if 0
            printf("TCP/IP connect: %s\n", buf);
            send(client, "ACK", 3, 0);
            unimplemented(client);
#endif
            timer = time(NULL);
            memset(filename, 0, sizeof(filename));
            UTC2file(timer, filename, sizeof(filename));
            if(print) msg_print(_print_buf, _print_bsize, "TCP/IP connect[%d]: %s SN:%s\n", (int)numchars, buf, device->sn);
            //printf("TCP/IP[%d] connect[%d]: %s SN:%s\n", device->socket, (int)numchars, buf, device->sn);
            //printf("TCP/IP connect[%d]: %s SN:%s\n", (int)numchars, buf, device->sn);
            //if(0!=decode_server(&print, _agree_obd, (const uint8_t *)buf, numchars, msg_buf, sizeof(msg_buf), client, csend))
            pthread_mutex_lock(&obd_mutex);
            //if(0!=decode_server(&print, _agree_obd, (const uint8_t *)buf, numchars, msg_buf, sizeof(msg_buf), device, csend))
#if 0
            if(0!=decode_server(&print, _agree_obd, (const uint8_t *)data, numchars-len, msg_buf, sizeof(msg_buf), device, csend))
                //if(0!=decode_server(&print, _agree_obd, (const uint8_t *)buf, numchars, msg_cache->data, DEVICE_DATA_SIZE, device, csend))
            {
                numchars2 = numchars;
                pthread_mutex_unlock(&obd_mutex);
                if(numchars2<512)
                {
                    //usleep(1000*10);   // 10ms
                    usleep(1000*10);   // 10ms
                    continue;  // 一次没有接受完，继续接收
                }
                numchars2 = 0; // 切换请求
                goto next;
            }
#else
            memset(&_ofp_data, 0, sizeof(_ofp_data));
            switch(device->protocol) // 判断协议类型
            {
                case PRO_TYPE_CCU:    // CCU
                    decode = -1;
                    break;
                case PRO_TYPE_YJ:    // YunJing
                    msg_print(_print_buf, _print_bsize, "协议类型：云景OBD协议 device->protocol:%d\n", device->protocol); fflush(stdout);
                    decode = _obd_obj->fops->decode_server(_obd_obj, (const uint8_t *)data, numchars-len, msg_buf, sizeof(msg_buf), &_ofp_data, &_print);
                    //ret = conn->_obd_obj->fops->decode_server(conn->_obd_obj, (uint8_t*)msg.c_str(), (uint16_t)msg.size(), msg_buf, sizeof(msg_buf), &_ofp_data, &_print);
                    if(0==decode) device->protocol = PRO_TYPE_YJ;
                    break;
                case PRO_TYPE_SHH:    // ShangHai
                    msg_print(_print_buf, _print_bsize, "协议类型：上海OBD协议 device->protocol:%d\n", device->protocol); fflush(stdout);
                    decode = _obd_obj->fops->decode_server(_obd_obj, (const uint8_t *)data, numchars-len, msg_buf, sizeof(msg_buf), &_ofp_data, &_print);
                    if(0==decode) device->protocol = PRO_TYPE_SHH;
                    break;
                default:
                    // 逐个协议遍历
                    msg_print(_print_buf, _print_bsize, "逐个协议遍历 device->protocol:%d\n", device->protocol); fflush(stdout);
                    decode = _obd_obj->fops->decode_server(_obd_obj, (const uint8_t *)data, numchars-len, msg_buf, sizeof(msg_buf), &_ofp_data, &_print);
                    if(0==decode) device->protocol = PRO_TYPE_SHH;
                    if(0!=decode)
                    {
                        msg_print(_print_buf, _print_bsize, "逐个协议遍历 device->protocol:%d\n", device->protocol); fflush(stdout);
                        //decode = decode_server(&print, _agree_obd_yj, (const uint8_t *)data, numchars-len, msg_buf, sizeof(msg_buf), device, csend, _print_buf, _print_bsize);
                        decode = _obd_obj->fops->decode_server(_obd_obj, (const uint8_t *)data, numchars-len, msg_buf, sizeof(msg_buf), &_ofp_data, &_print);
                        if(0==decode) device->protocol = PRO_TYPE_YJ;
                    }
                    break;
            }
            if(_ofp_data._tlen>10)
            {
                //printf("@%s-%d Send to client: %3d:%s\n", __func__, __LINE__, _ofp_data._tlen, _ofp_data._tbuf);
                //csend(_ofp_data._tbuf, _ofp_data._tlen);
                    send(device->socket, buf, len, 0);
            }
            if(0!=decode)
            {
                numchars2 = numchars;
                pthread_mutex_unlock(&obd_mutex);
                if(numchars2<512)
                {
                    usleep(1000*10);   // 10ms
                    continue;  // 一次没有接受完，继续接收
                }
                numchars2 = 0; // 切换请求
                goto next;
            }
#endif
            pthread_mutex_unlock(&obd_mutex);
            device->last_time = time(NULL);
            //msg_cache->write++;
            //if(msg_cache->write>=DEVICE_ITEM_SIZE) msg_cache->write = 0;
            numchars2 = 0;
            if(save_log)
            {
                save_to_file(timer, buf, numchars);
            }
            //decode_server(_agree_obd, (const uint8_t *)buf, numchars, msg_buf, sizeof(msg_buf), client, csend);
            goto next;
        }
        printf("httpd connect\n");

        if (strcasecmp(method, "POST") == 0)
            cgi = 1;

        i = 0;
        while (ISspace(buf[j]) && (j < numchars))
            j++;
        while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < numchars))
        {
            url[i] = buf[j];
            i++; j++;
        }
        url[i] = '\0';

        if (strcasecmp(method, "GET") == 0)
        {
            query_string = url;
            while ((*query_string != '?') && (*query_string != '\0'))
                query_string++;
            if (*query_string == '?')
            {
                cgi = 1;
                *query_string = '\0';
                query_string++;
            }
        }

        sprintf(path, "htdocs%s", url);
        if (path[strlen(path) - 1] == '/')
            strcat(path, "index.html");
        if (stat(path, &st) == -1) {
            while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
                numchars = get_line(device->socket, buf, sizeof(buf));
            not_found(device->socket);
        }
        else
        {
            if ((st.st_mode & S_IFMT) == S_IFDIR)
                strcat(path, "/index.html");
            if ((st.st_mode & S_IXUSR) ||
                    (st.st_mode & S_IXGRP) ||
                    (st.st_mode & S_IXOTH)    )
                cgi = 1;
            if (!cgi)
                serve_file(device->socket, path);
            else
                execute_cgi(device->socket, path, method, query_string);
        }
next:
        usleep(1000*10);   // 10ms delay
        // 一个线程处理多个请求
#if 0
        if((tasks_max<32) && (0==pthread_mutex_trylock(&list_task_mutex)))
        {
            if(!list_empty(&list_task))  // 获取新的任务
            {
                struct list_head* new_task=NULL;
                new_task = list_task.next; // 取出任务
                list_del(new_task);
                device = container_of(new_task, struct device_list, list_work);
                list_add(&(device->list_work), &tasks);
                tasks_max++;
            }
            pthread_mutex_unlock(&list_task_mutex);
        }
#endif
        tasks_max = new_task(tasks_max, &list_task, &tasks, &list_task_mutex);
        if(list_empty(&tasks))  // 无需要处理的任务
        {
            printf("无需要处理的任务\n");
            break;
        }
        //        // 切换请求, move node
        //        node = tasks.prev;
        //        list_del(node);
        //        list_add(node, &tasks);
        //        while(node==(&tasks)) node = node->next;
        //        device = container_of(node, struct device_list, list_work);
        device = new_request(&tasks);
        //client = device->socket;
        memset(buf, 0, sizeof (buf));
        timeout=0;
        numchars2 = 0;
    }
    //printf("\n连接断开\n");
    printf("\n线程等待\n");
}
void accept_request(void *arg)
{
    //const char viewer[] = "SocketViewer";
    //const struct agreement_ofp* _agree_obd=NULL;
    //uint8_t msg_buf[4096];
    //	int client = 0;//(intptr_t)arg;
    //char buf[1024];
    //	size_t numchars;
    //	size_t numchars2;
    //	char method[255];
    //	char filename[255];
    //	clock_t start, finish;
    //	double  duration;
    //	int recv_status=1;
    //	time_t time2, time1;
    //	FILE* fd;
    //	char url[255];
    //	char path[512];
    //	size_t i, j;
    //	time_t timer;
    //	struct stat st;
    volatile struct device_list* device=NULL;
    //struct device_data  *msg_cache;
    //int cgi = 0;      /* becomes true if server decides this is a CGI
    //          * program */
    //char *query_string = NULL;
    //struct list_head tasks; // 任务链表
    //int tasks_max;          // 任务数
    struct list_head* node;
    //int print=0;

    //tasks_max = 0;
    if(NULL == arg) // 参数错误
    {
        printf("%s@%d arg=NULL\n", __func__, __LINE__);
        return;
    }
    pthread_lock();
    request_count++;
    device = (struct device_list*)arg;
    device->save_log = 1;
    device->relay_fd = -1;
    online_thread_add(device);
    //INIT_LIST_HEAD(&tasks);
    if(0==list_task_init)
    {
        list_task_init=1;
        INIT_LIST_HEAD(&list_task);
        INIT_LIST_HEAD(&list_trunk);
        pthread_mutex_init(&list_task_mutex, NULL);
        pthread_mutex_init(&obd_mutex, NULL);
        pthread_mutex_init(&trunk_mutex, NULL);
    }
    pthread_unlock();
    pthread_mutex_lock(&list_task_mutex);
    list_add(&(device->list_work), &list_task);
    pthread_mutex_unlock(&list_task_mutex);
    sleep(1);  // 让出任务
    pthread_mutex_lock(&trunk_mutex);
    if(!list_empty(&list_trunk)) // 启动中继线程
    {
        node = list_trunk.next; // 取出任务
        list_del(node);
        device = container_of(node, struct device_list, list_work);
        pthread_mutex_unlock(&trunk_mutex);
        thread_trunking(device);
        return;
    }
    pthread_mutex_unlock(&trunk_mutex);
    if(0!=pthread_mutex_trylock(&list_task_mutex)) return ; // 返回
    if(list_empty(&list_task))  // 其它线程在处理该任务
    {
        pthread_mutex_unlock(&list_task_mutex);
        printf("%s@%d list_task is empty\n", __func__, __LINE__);
        return ; // 返回
    }
    node = list_task.next; // 取出任务
    list_del(node);
    device = container_of(node, struct device_list, list_work);
    pthread_mutex_unlock(&list_task_mutex);
    thread_request(device);  // 处理接收请求
}

/**********************************************************************/
/* Inform the client that a request it has made has a problem.
 * Parameters: client socket */
/**********************************************************************/
void bad_request(int client)
{
#if GCC_BUILD
    char buf[1024];

    sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "<P>Your browser sent a bad request, ");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "such as a POST without a Content-Length.\r\n");
    send(client, buf, sizeof(buf), 0);
#else
    (void)client;
#endif
}

/**********************************************************************/
/* Put the entire contents of a file out on a socket.  This function
 * is named after the UNIX "cat" command, because it might have been
 * easier just to do something like pipe, fork, and exec("cat").
 * Parameters: the client socket descriptor
 *             FILE pointer for the file to cat */
/**********************************************************************/
void cat(int client, FILE *resource)
{
    char buf[1024];

    fgets(buf, sizeof(buf), resource);
    while (!feof(resource))
    {
#if GCC_BUILD
        send(client, buf, strlen(buf), 0);
#endif
        fgets(buf, sizeof(buf), resource);
    }
}

/**********************************************************************/
/* Inform the client that a CGI script could not be executed.
 * Parameter: the client socket descriptor. */
/**********************************************************************/
void cannot_execute(int client)
{
#if GCC_BUILD
    char buf[1024];

    sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
    send(client, buf, strlen(buf), 0);
#endif
}

/**********************************************************************/
/* Print out an error message with perror() (for system errors; based
 * on value of errno, which indicates system call errors) and exit the
 * program indicating an error. */
/**********************************************************************/
void error_die(const char *sc)
{
    perror(sc);
    exit(1);
}

/**********************************************************************/
/* Execute a CGI script.  Will need to set environment variables as
 * appropriate.
 * Parameters: client socket descriptor
 *             path to the CGI script */
/**********************************************************************/
void execute_cgi(int client, const char *path,
                 const char *method, const char *query_string)
{
#if GCC_BUILD
    char buf[1024];
    int cgi_output[2];
    int cgi_input[2];
    pid_t pid;
    int status;
    int i;
    char c;
    int numchars = 1;
    int content_length = -1;

    buf[0] = 'A'; buf[1] = '\0';
    if (strcasecmp(method, "GET") == 0)
        while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
            numchars = get_line(client, buf, sizeof(buf));
    else if (strcasecmp(method, "POST") == 0) /*POST*/
    {
        numchars = get_line(client, buf, sizeof(buf));
        while ((numchars > 0) && strcmp("\n", buf))
        {
            buf[15] = '\0';
            if (strcasecmp(buf, "Content-Length:") == 0)
                content_length = atoi(&(buf[16]));
            numchars = get_line(client, buf, sizeof(buf));
        }
        if (content_length == -1) {
            bad_request(client);
            return;
        }
    }
    else/*HEAD or other*/
    {
    }


    if (pipe(cgi_output) < 0) {
        cannot_execute(client);
        return;
    }
    if (pipe(cgi_input) < 0) {
        cannot_execute(client);
        return;
    }

    if ( (pid = fork()) < 0 ) {
        cannot_execute(client);
        return;
    }
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    if (pid == 0)  /* child: CGI script */
    {
        char meth_env[255];
        char query_env[255];
        char length_env[255];

        dup2(cgi_output[1], STDOUT);
        dup2(cgi_input[0], STDIN);
        close(cgi_output[0]);
        close(cgi_input[1]);
        sprintf(meth_env, "REQUEST_METHOD=%s", method);
        putenv(meth_env);
        if (strcasecmp(method, "GET") == 0) {
            sprintf(query_env, "QUERY_STRING=%s", query_string);
            putenv(query_env);
        }
        else {   /* POST */
            sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
            putenv(length_env);
        }
        //execl(path, NULL);
        execl(path, path, NULL);
        exit(0);
    } else {    /* parent */
        close(cgi_output[1]);
        close(cgi_input[0]);
        if (strcasecmp(method, "POST") == 0)
            for (i = 0; i < content_length; i++) {
                recv(client, &c, 1, 0);
                write(cgi_input[1], &c, 1);
            }
        while (read(cgi_output[0], &c, 1) > 0)
            send(client, &c, 1, 0);

        close(cgi_output[0]);
        close(cgi_input[1]);
        waitpid(pid, &status, 0);
    }
#endif
}

/**********************************************************************/
/* Get a line from a socket, whether the line ends in a newline,
 * carriage return, or a CRLF combination.  Terminates the string read
 * with a null character.  If no newline indicator is found before the
 * end of the buffer, the string is terminated with a null.  If any of
 * the above three line terminators is read, the last character of the
 * string will be a linefeed and the string will be terminated with a
 * null character.
 * Parameters: the socket descriptor
 *             the buffer to save the data in
 *             the size of the buffer
 * Returns: the number of bytes stored (excluding null) */
/**********************************************************************/
#if 0
int get_line(int sock, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;

    while ((i < size - 1) && (c != '\n'))
    {
        //n = recv(sock, &c, 1, 0);
        n = recv(sock, &c, 1, MSG_DONTWAIT);
        /* DEBUG printf("%02X\n", c); */
        if (n > 0)
        {
            /*if(n!=sizeof(c))
              {
              c = '\n';
              continue;
              }*/
            if (c == '\r')
            {
                n = recv(sock, &c, 1, MSG_PEEK);
                /* DEBUG printf("%02X\n", c); */
                if ((n > 0) && (c == '\n'))
                    recv(sock, &c, 1, 0);
                else
                    c = '\n';
            }
            buf[i] = c;
            i++;
        }
        else
            c = '\n';
    }
    buf[i] = '\0';

    return(i);
}
#endif

int read_threads(int sock, char *buf, int size, int *status)
{
    int i = 0;
    char c = '\0';
    int n;

    //while ((i < size - 1) && (c != '\n'))
    while ((i < size - 1))
    {
        //n = recv(sock, &c, 1, 0);
#if GCC_BUILD
        n = recv(sock, &c, 1, MSG_DONTWAIT);
#else
        n=0;
#endif
        /* DEBUG printf("%02X\n", c); */
        if (n > 0)
        {
            buf[i] = c;
            i++;
        }
        else
            break;
    }
    buf[i] = '\0';
    *status = n;
    return(i);
}
int get_line(int sock, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;

    //while ((i < size - 1) && (c != '\n'))
    while ((i < size - 1))
    {
#if GCC_BUILD
        //n = recv(sock, &c, 1, 0);
        n = recv(sock, &c, 1, MSG_DONTWAIT);
#else
        n=0;
#endif
        /* DEBUG printf("%02X\n", c); */
        if (n > 0)
        {
            buf[i] = c;
            i++;
        }
        else
            break;
    }
    buf[i] = '\0';
    //recv_status = n;
    return(i);
}

/**********************************************************************/
/* Return the informational HTTP headers about a file. */
/* Parameters: the socket to print the headers on
 *             the name of the file */
/**********************************************************************/
void headers(int client, const char *filename)
{
#if GCC_BUILD
    char buf[1024];
    (void)filename;  /* could use filename to determine file type */

    strcpy(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
#else
    (void)client;
    (void)filename;
#endif
}

/**********************************************************************/
/* Give a client a 404 not found status message. */
/**********************************************************************/
void not_found(int client)
{
#if GCC_BUILD
    char buf[1024];

    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "your request because the resource specified\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
#endif
}

/**********************************************************************/
/* Send a regular file to the client.  Use headers, and report
 * errors to client if they occur.
 * Parameters: a pointer to a file structure produced from the socket
 *              file descriptor
 *             the name of the file to serve */
/**********************************************************************/
void serve_file(int client, const char *filename)
{
    FILE *resource = NULL;
    int numchars = 1;
    char buf[1024];

    buf[0] = 'A'; buf[1] = '\0';
    while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
        numchars = get_line(client, buf, sizeof(buf));

    resource = fopen(filename, "r");
    if (resource == NULL)
        not_found(client);
    else
    {
        headers(client, filename);
        cat(client, resource);
    }
    fclose(resource);
}

/**********************************************************************/
/* Inform the client that the requested web method has not been
 * implemented.
 * Parameter: the client socket */
/**********************************************************************/
void unimplemented(int client)
{
#if GCC_BUILD
    char buf[1024];

    sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</TITLE></HEAD>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
#endif
}


