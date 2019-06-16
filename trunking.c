#include "trunking.h"
#include "thread_list.h"
#include "agreement/storage_pack.h"
#include <unistd.h>
#include <string.h>

#define LINUX_BUILD   1
#if LINUX_BUILD
#include <sys/socket.h>
#endif

static int trunk_flag = 0;
void trunking_exit(void)
{
    trunk_flag = 0;
}

static char buffer[4096];
static void trunking_call(const int client, const struct device_list* const device, const uint8_t pack[], const uint16_t _psize)
{
#if LINUX_BUILD
    json_device(buffer, sizeof(buffer), device, pack, _psize);
    send(client, buffer, strlen(buffer), 0);
    //printf("send:%s\n", buffer);
#else
    (void)client;
    json_device(buffer, sizeof(buffer), device, pack, _psize);
#endif
}

void trunking(const int client)
{
    trunk_flag = 1;
    while(trunk_flag)
    {
        online_thread_for_each(client, trunking_call);
        usleep(1000*50);
    }
}
