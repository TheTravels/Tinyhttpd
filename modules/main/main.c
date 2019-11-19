/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : parameter.c
* Author             : Merafour
* Last Modified Date : 11/16/2019
* Description        : 命令行参数处理.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <time.h>
#include <getopt.h>
#include <pwd.h>
#include <signal.h>
#include <sys/param.h>
#include <syslog.h>
#include <dirent.h>
#include <execinfo.h>
#include "../agreement/agreement.h"
#include "../obd/json_list.h"
//#include "../obd/thread_list.h"
#include "lock.h"
#include "sql.h"
#include "modules/config/config_data.h"
//#include "DateTime.h"
#include "../obd/thread_vin.h"
#include "accept_request.h"
#include "../epoll/epoll.h"
#include "thread_pool.h"
#include "../epoll/epoll_server.h"

//#define ISspace(x) isspace((int)(x))

//#define SERVER_STRING "Server: jdbhttpd/0.1.0\r\n"
//#define STDIN   0
//#define STDOUT  1
//#define STDERR  2

extern int cmd_parameter(int argc, char *argv[]);
extern void epoll_pthread_init(const int _thread_max);
extern int save_log;
extern int relay;

/**********************************************************************/
/* This function starts the process of listening for web connections
 * on a specified port.  If the port is 0, then dynamically allocate a
 * port and modify the original port variable to reflect the actual
 * port.
 * Parameters: pointer to variable containing the port to connect on
 * Returns: the socket */
/**********************************************************************/
int startup(u_short *port)
{
	int httpd = 0;
	int on = 1;
	struct sockaddr_in name;

	httpd = socket(PF_INET, SOCK_STREAM, 0);
	if (httpd == -1)
		error_die("socket");
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(*port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	if ((setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)  
	{  
		error_die("setsockopt failed");
	}
	if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
		error_die("bind");
	if (*port == 0)  /* if dynamically allocating a port */
	{
		socklen_t namelen = sizeof(name);
		if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1)
			error_die("getsockname");
		*port = ntohs(name.sin_port);
	}
	if (listen(httpd, 5) < 0)
		error_die("listen");
	return(httpd);
}

/*static void UTC2file(const uint32_t times, void* const buf, const size_t _size)
{
	DateTime      utctime   = {.year = 1970, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 0};
	DateTime      localtime = {.year = 1970, .month = 1, .day = 1, .hour = 8, .minute = 0, .second = 0};
	struct passwd *npwd;
	npwd = getpwuid(getuid());
	//printf("当前登陆的用户名为：%s\n", npwd->pw_name);
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
	snprintf((char *)buf, (size_t)_size, "./daemon/server-%s-%d-%.2d-%.2d-%02d%02d%02d.txt", npwd->pw_name, localtime.year, localtime.month, localtime.day, localtime.hour, localtime.minute, localtime.second);
}*/

void do_backtrace(void)
{
    void *array[100];
    char **strings;
    int size, i;
    size = backtrace(array, 100);
    strings = backtrace_symbols(array, size);
    printf("%p\n", strings);
    for(i = 0; i < size; i++)
        printf("sigsegv at :%p:%s\n", array[i], strings[i]);
    free(strings);
}

void when_sigsegv()
{
    do_backtrace();
    //	int i = find_index((long)pthread_self());
    //	printf("sigsegv...%d...\n", i);
    //	siglongjmp(env[i], MAX);
}

void daemon_thread(void *arg)
{
	(void)arg;
	while(1)
	{
		sleep(100);  // 100s
		get_fw();
	}
}

int main(int argc, char *argv[])
{
	int server_sock = -1;
	//u_short port = 4000;
    u_short port = 9910;
    //int client_sock = -1;
    //struct sockaddr_in client_name;
    //socklen_t  client_name_len = sizeof(client_name);
	pthread_t newthread;
	pthread_t vinthread;
    //char daemon=0;
    //struct device_list* device;
	char pwd[128] ;
	struct passwd *npwd;
    struct epoll_obj* _epoll_listen=NULL;
    char _epoll_listen_buf[sizeof(struct epoll_obj)];
    struct local_config_data* _cfg_data = NULL;
	npwd = getpwuid(getuid());
    memset(pwd, 0, sizeof(pwd));
    getcwd(pwd, sizeof(pwd));
    //printf("当前登陆的用户名为：%s\n", npwd->pw_name);
    printf("启动目录:%s\n", pwd);
    //printf("[%s-%d] _cfg_path:%s\n", __func__, __LINE__, _local_config_data->_cfg_path);
    local_config_data_init(pwd);
    port = cmd_parameter(argc, argv);
    _local_config_data->load(_local_config_data);
	thread_vin_init(port);
    //server_log_init(port);
    _cfg_data = (struct local_config_data*)_local_config_data->data;
    //vin_list_load(_cfg_data->vinList); // vin_list_load("./upload/vin.list");
    obd_agree_obj_yunjing.fops->base->vin.load(_cfg_data->vinList);  // 加载 VIN 码文件
#if 0
	fflush(stdout);
	setvbuf(stdout,NULL,_IONBF,0);
    //UTC2file(time(NULL), pwd, sizeof (pwd));
	printf("test stdout\n");
    freopen("test1.txt","w",stdout); //注: 不要使用这类的代码 stdout = fopen("test1.txt","w");   这样的话输出很诡异的. 最好使用  freopen 这类的函数来替换它.
    //freopen(pwd,"w",stdout); //注: 不要使用这类的代码 stdout = fopen("test1.txt","w");   这样的话输出很诡异的. 最好使用  freopen 这类的函数来替换它.
	printf("test file\n");
	//freopen("/dev/tty","w",stdout);
	//printf("test tty\n");
	//#else
	memset(pwd, 0, sizeof(pwd));
    //UTC2file(time(NULL), pwd, sizeof (pwd));
	fflush(stdout);
	//setvbuf(stdout,NULL,_IONBF,0);
	//printf("test stdout\n");
	//int save_fd = dup(STDOUT_FILENO); // 保存标准输出 文件描述符 注:这里一定要用 dup 复制一个文件描述符. 不要用 = 就像是Winodws下的句柄.
	dup(STDOUT_FILENO); // 保存标准输出 文件描述符 注:这里一定要用 dup 复制一个文件描述符. 不要用 = 就像是Winodws下的句柄.
    int fd = open("test1.txt",(O_RDWR | O_CREAT), 0644);
    //int fd = open(pwd, (O_RDWR | O_CREAT), 0644);
	dup2(fd,STDOUT_FILENO); // 用我们新打开的文件描述符替换掉 标准输出
	//printf("test file\n");
#endif
	//init_signals();
    //signal(SIGSEGV, when_sigsegv);
	/*struct passwd *npwd;
	npwd = getpwuid(getuid());
	printf("当前登陆的用户名为：%s\n", npwd->pw_name);*/
	//if(1==daemon) init_daemon();
	memset(pwd, 0, sizeof(pwd));
	//char *p = getcwd(pwd, sizeof(pwd));
	getcwd(pwd, sizeof(pwd));
	//printf("pwd:%s   p:%s size:%d  \n", pwd, p, strlen(pwd));
	printf("当前登陆的用户名为：%s\n", npwd->pw_name);
	printf("Working Directory:%s\n", pwd);
    //if(1==daemon) reset();

	thread_list_init();
	server_sock = startup(&port);
	printf("\nhttpd running on port %d\n", port);
	if(9910==port) relay = 0;
	else relay = 1;
	//printf("\n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n\n");
	printf("\n\n\n");
	fflush(stdout);
	pthread_lock_init();
	//pool_init (1024); 
	//pool_init (128); 
	get_fw();
	pthread_create(&newthread , NULL, (void *)daemon_thread, NULL);
    pthread_create(&vinthread , NULL, (void *)thread_get_vin, NULL);
    //pool_init (8);
    epoll_pthread_init(2);

    printf("[%s-%d] \n", __func__, __LINE__);
    _epoll_listen = epoll_listen_init(_epoll_listen_buf);
    //printf("[%s-%d] \n", __func__, __LINE__);
    if(NULL==_epoll_listen)
    {
        printf("[%s-%d] listen fail!\n", __func__, __LINE__);
        exit(0);
    }
    while(1)
    {
        //printf("[%s-%d] \n", __func__, __LINE__);
        _epoll_listen->do_epoll(_epoll_listen, server_sock);
        //printf("[%s-%d] \n", __func__, __LINE__);
        //sleep(10);
    }
#if 0
	while (1)
	{
		//pthread_attr_t attr;
		client_sock = accept(server_sock,
				(struct sockaddr *)&client_name,
				&client_name_len);
		if (client_sock == -1)
			error_die("accept");
		device = get_thread_free();
		if(NULL == device)
		{
			printf("get_thread_free fail!\n");
			close(client_sock);
			continue;
		}
#if 0
		/* accept_request(&client_sock); */
		if (pthread_create(&newthread , NULL, (void *)accept_request, (void *)(intptr_t)client_sock) != 0)
			perror("pthread_create");
#else
		device->socket = client_sock;
		printf("device->socket:%d\n", device->socket);
#if 0
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);       //因为你的线程不便于等待的关系，设置为分离线程吧 
		//if (pthread_create(&newthread , NULL, (void *)accept_request, (void *)device) != 0)
		if (pthread_create(&newthread , &attr, (void *)accept_request, (void *)device) != 0)
			perror("pthread_create");
#else
        pool_add_worker(accept_request, device);
#endif
#endif
	}
#endif
    //close(server_sock);

	return(0);
}
