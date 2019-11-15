/**
 * 中继
 */
#include "trunking.h"
#include "thread_list.h"
#include "agreement/storage_pack.h"
#include <unistd.h>
#include <string.h>

#ifndef GCC_BUILD
#define GCC_BUILD   0
#endif
#if GCC_BUILD
#include <sys/socket.h>

#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "DateTime.h"
#include <stdio.h>
void trunking_utc(const time_t times, void* const buf, const size_t _size)
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
    snprintf((char *)buf, (size_t)_size, "%d-%.2d-%.2d-%02d%02d%02d", localtime.year, localtime.month, localtime.day, localtime.hour, localtime.minute, localtime.second);
}

static int trunk_flag = 0;
void trunking_exit(void)
{
    trunk_flag = 0;
}

struct trunking_data{
    const int client;
    uint16_t read;   // 数据中继计数
    uint16_t exit;
};

static char buffer[4096];
static void trunking_call(void* client, const struct device_list* const device, const uint8_t pack[], const uint16_t _psize)
{
    int len = 0;
//    char utc[128];
//    memset(utc, 0, sizeof (utc));
//    trunking_utc(time(NULL), utc, sizeof(utc));
    struct trunking_data *trunk = (struct trunking_data*) client;
    //printf("trunking_call Time[0x%08X] [ %d | %d ]: %s \n", device, trunk->read, device->write, utc);
    printf("trunking_call [0x%08X] [ %d | %d ] VIN:%s SN:%s \n", device, trunk->read, trunk->exit, device->VIN, device->sn);
    //if(trunk->read==device->write) return;
    //trunk->read = device->write;
    trunk->read++;
    if(trunk->exit) return;
    json_device(buffer, sizeof(buffer), device, pack, _psize);
    len = send(trunk->client, buffer, strlen(buffer)+1, 0);  // send '\0'
    //printf("trunking_call send[ %ld | %d ]\n", strlen(buffer), len);
    if(len<0)
    {
        printf("trunking thread exit\n"); fflush(stdout);
        //pthread_exit(NULL);
        trunk->exit = 1;
    }
    //printf("send:%s\n", buffer);
}

static int read_threads(int sock, char *buf, int size, int *status)
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
        *status = n;
        return(i);
}

// 每次处理一个请求
int trunking(const int client)
{
    size_t numchars;
    char buf[128];
    int recv_status=1;
    //const char download[]="Download";
    struct trunking_data trumk={client, 0};
    trunk_flag = 1;

    //while(trunk_flag)
    //while(1)
    {
        memset(buf, 0, sizeof(buf));
        numchars = read_threads(client, buf, sizeof(buf), &recv_status);
        if(0==recv_status)
        {
            close(client);
            //pthread_exit(NULL);
            //break;
            return -1;
        }
        /*if(0==memcmp(buf, download, sizeof(download)))
        {
            online_thread_for_each(client, trunking_call);
        }*/
        if(numchars>0) online_thread_for_each(&trumk, trunking_call);
        if(trumk.exit) return -1;
        //usleep(1000*500);
        //sleep(1);
    }
    return 0;
}
