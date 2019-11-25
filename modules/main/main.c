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
extern char daemon_run;

struct fork_process_data {
    pid_t pid;          // 进程 ID
    uint16_t port;      // 侦听端口
    timer_t _time;
    int reset_time;     // 重启时间
};
static struct fork_process_data _fork_process[128];
static const uint16_t _fork_process_size = sizeof(_fork_process)/sizeof(_fork_process[0]);

/**********************************************************************/
/* This function starts the process of listening for web connections
 * on a specified port.  If the port is 0, then dynamically allocate a
 * port and modify the original port variable to reflect the actual
 * port.
 * Parameters: pointer to variable containing the port to connect on
 * Returns: the socket */
/**********************************************************************/
static int startup(u_short *port)
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

#if 0
int fork_process(const int _process_size)
{
    int i;
    pid_t pid;
    for(i=0; i<_process_size; i++)
    {
        if(i>_fork_process_size) break;
        if(_fork_process[i].port<1024) break;
        pid = fork();
        if(pid < 0)
        {
            perror("fork fail ");
            exit(1);
        }
        else if(pid == 0) //子进程
        {
            sleep(1);
            return i;
        }
        else  //父进程
        {
            _fork_process[i].pid = pid;
        }
    }
    return -1; //父进程
}
#endif
uint16_t fork_process(struct fork_process_data* const _process)
{
    pid_t pid;
    if(_process->port<1024) return 0; //父进程
    pid = fork();
    if(pid < 0)
    {
        perror("fork fail ");
        //exit(1);
        sleep(1);
        return 0; //父进程
    }
    else if(pid == 0) //子进程
    {
        sleep(1);
        return _process->port;
    }
    else  //父进程
    {
        _process->pid = pid;
    }
    return 0; //父进程
}
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>
#include <stdio.h>

/*seconds: the seconds; mseconds: the micro seconds*/
void setTimer(int seconds, int mseconds)
{
    struct timeval temp;
    temp.tv_sec = seconds;
    temp.tv_usec = mseconds;
    select(0, NULL, NULL, NULL, &temp);
    //printf("timer\n");
    //return ;
}
extern char **environ;

void my_initproctitle(char* argv[], char** last)
{
    int i = 0;
    char* p_tmp = NULL;
    size_t i_size = 0;
    for(i = 0; environ[i]; i++){
        i_size += strlen(environ[i]) + 1;
    }
    p_tmp = malloc(i_size);
    if(p_tmp == NULL){
        return ;
    }
    *last = argv[0];
    for(i = 0; argv[i]; i++){
        *last += strlen(argv[i]) + 1;
    }
    for(i = 0; environ[i]; i++){
        i_size = strlen(environ[i]) + 1;
        *last += i_size;
        strncpy(p_tmp, environ[i], i_size);
        environ[i] = p_tmp;
        p_tmp += i_size;
    }
    (*last)--;
    return ;
}
void my_setproctitle(char* argv[], char** last, char* title)
{
    char* p_tmp = NULL;
    /* argv[1] = NULL; */
    p_tmp = argv[0];
    /* memset(p_tmp, 0, *last - p_tmp); */
    strncpy(p_tmp, title, *last - p_tmp);
    return ;
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
    char title[128];
    char* p_last = NULL;
	npwd = getpwuid(getuid());
    memset(pwd, 0, sizeof(pwd));
    getcwd(pwd, sizeof(pwd));
    //printf("当前登陆的用户名为：%s\n", npwd->pw_name);
    printf("启动目录:%s\n", pwd);
    //printf("[%s-%d] _cfg_path:%s\n", __func__, __LINE__, _local_config_data->_cfg_path);
    //local_config_data_init(pwd);
    port = cmd_parameter(argc, argv);
    //_local_config_data->load(_local_config_data);
	thread_vin_init(port);
    //server_log_init(port);
    _cfg_data = (struct local_config_data*)_local_config_data->data;
    // 创建子进程
    if(1==daemon_run)
    {
        int i;
        int _process_size=0;
        timer_t _time;
        struct host_listening*  _server=NULL;
        struct fork_process_data* _process = NULL;
        _time = time(NULL);
        memset(_fork_process, 0, sizeof(_fork_process));
        printf("[%s-%d] 创建子进程 _time: %d \n", __func__, __LINE__, _time);
        for(i=0; i<_fork_process_size; i++)
        //for(i=0; i<10; i++)
        {
            _server=&_cfg_data->local_list[i];
            //printf("[%s-%d] _server->port:%d \n", __func__, __LINE__, _cfg_data->local_list[i].port);
            //printf("server%d host:%s, port:%d\n", i, _cfg_data->local_list[i].host, _cfg_data->local_list[i].port);
            if(_server->port<1024) break;
            _process = &_fork_process[i];
            _process->port = _server->port;
            _process->reset_time = (i+127)+600*i+(3600*3); // 重启时间错开
            _process->_time = _time + _process->reset_time; // 设置重启时间
        }
        //_fork_process[i++].port = port;
        _process = &_fork_process[i++];
        _process->port = port;
        _process->reset_time = (i+127)+600*i+(3600*3); // 重启时间错开
        _process->_time = _time + _process->reset_time; // 设置重启时间
        _process_size=i;
        printf("[%s-%d] 子进程数[%d]\n", __func__, __LINE__, i);
        // fork
        for(i=0; i<_process_size; i++)
        {
            port = fork_process(&_fork_process[i]);
            if(port>1024) break; //子进程
        }
        //index = fork_process(i);
        if(port<=1024)
        {
            printf("[%s-%d] 父进程端口[%d]: %d \n", __func__, __LINE__, index, port);
            while(port<=1024)
            {
                pid_t pid;
                setTimer(5, 0); // sleep 10s
                _time = time(NULL);
                printf("[%s-%d] 父进程 _time: %d \n", __func__, __LINE__, _time);
                port = 0;
                for(i=0; i<_process_size; i++)
                {
                    pid=0;
                    _process = &_fork_process[i];
                    if(_process->pid>0) pid = waitpid(_process->pid, NULL, WNOHANG);
                    if((_time > _process->_time) || (pid>0)) // timeout, reset
                    {
                        if(0==pid)
                        {
                            kill(_process->pid, SIGKILL);//杀死 pid 发送进程的信号,kill 给其他进程发送信号，指定进程号
                            waitpid(_process->pid, NULL, 0); //等待回收子进程的资源
                        }
                        _process->pid = 0;
                        setTimer(1, 0); // sleep 1s
                        _process->_time = _time + _process->reset_time; // 设置重启时间
                        port = fork_process(_process);
                        if(port>1024) //子进程
                        {
                            printf("[%s-%d] 子进程端口: %d \n", __func__, __LINE__, port);
                            break;
                        }
                    }
                }
            }
        }
        else //子进程
        {
            //port = _fork_process[index].port;
            printf("[%s-%d] 子进程端口: %d \n", __func__, __LINE__, port);
        }
    }
    memset(title, 0, sizeof(title));
    snprintf(title, sizeof(title), "%s process[%d]", argv[0], port);
    my_initproctitle(argv, &p_last);
    my_setproctitle(argv, &p_last, title);
    //vin_list_load(_cfg_data->vinList); // vin_list_load("./upload/vin.list");
    obd_agree_obj_yunjing.fops->base->vin.load(_cfg_data->vinList);  // 加载 VIN 码文件
    //setTimer(20, 0); // sleep 10s
    //exit(0);
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
    //if(1==_cfg_data->_vin_cfg._turn_on) pthread_create(&vinthread , NULL, (void *)thread_get_vin, NULL);
    pthread_create(&vinthread , NULL, (void *)thread_get_vin, NULL);
    //pool_init (8);
    //epoll_pthread_init(_cfg_data->nCfgPthreadCounts_+(_cfg_data->_vin_cfg._turn_on&0x01));
    epoll_pthread_init(_cfg_data->nCfgPthreadCounts_);

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
