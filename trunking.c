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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#endif

static int trunk_flag = 0;
void trunking_exit(void)
{
    trunk_flag = 0;
}

static char buffer[4096];
static void trunking_call(const int client, const struct device_list* const device, const uint8_t pack[], const uint16_t _psize)
{
#if GCC_BUILD
    int len = 0;
    json_device(buffer, sizeof(buffer), device, pack, _psize);
    len = send(client, buffer, strlen(buffer), 0);
    if(len<0)
    {
        printf("trunking thread exit\n"); fflush(stdout);
        pthread_exit(NULL);
    }
    //printf("send:%s\n", buffer);
#else
    (void)client;
    json_device(buffer, sizeof(buffer), device, pack, _psize);
#endif
}

#if GCC_BUILD
static int read_threads(int sock, char *buf, int size, int *status)
{
        int i = 0;
        char c = '\0';
        int n;

        //while ((i < size - 1) && (c != '\n'))
        while ((i < size - 1))
        {
                //n = recv(sock, &c, 1, 0);
                n = recv(sock, &c, 1, MSG_DONTWAIT);
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
#endif

void trunking(const int client)
{
#if GCC_BUILD
    size_t numchars;
    char buf[128];
    int recv_status=1;
    const char download[]="Download";
#endif
    trunk_flag = 1;

    while(trunk_flag)
    {
#if GCC_BUILD
        memset(buf, 0, sizeof(buf));
        numchars = read_threads(client, buf, sizeof(buf), &recv_status);
        if(0==recv_status)
        {
            close(client);
            pthread_exit(NULL);
            break;
        }
        /*if(0==memcmp(buf, download, sizeof(download)))
        {
            online_thread_for_each(client, trunking_call);
        }*/
        online_thread_for_each(client, trunking_call);
        usleep(1000*50);
#else
        online_thread_for_each(client, trunking_call);
        usleep(1000*50);
#endif
    }
}
