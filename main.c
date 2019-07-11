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
#include <sys/wait.h>
#include <time.h>
#include <getopt.h>
#include "accept_request.h"
#include <pwd.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/param.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <syslog.h>
#include <dirent.h>

#include "agreement/agreement.h"
#include "json_list.h"
#include "thread_list.h"
#include "lock.h"

#define ISspace(x) isspace((int)(x))

#define SERVER_STRING "Server: jdbhttpd/0.1.0\r\n"
#define STDIN   0
#define STDOUT  1
#define STDERR  2

int startup(u_short *);


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


/**********************************************************************/

/* parse the command option 
 * -d(--daemon)   daemon process
 *  -p(--port)     assign http port
 *  -s(--port)     assign https port
 *  -l(--log)      log path
 *  */

static void usage(void)
{
	//fprintf(stderr,"usage:./main [-d --daemon] [-p --port] [-s --sslport] [-l --log] [-v --version] [-h --help]\n\n");
	fprintf(stderr,"usage:./build/httpd [-c --config] [-d --daemon] [-p --port] [-N --NULL] [-f --fliter] [-S --SN] [-l --log] [-L --list] [-v --version] [-h --help]\n\n");
	fprintf(stderr,"usage:./build/httpd -h\n");
	fprintf(stderr,"usage:./build/httpd -p 9910 -f VINABCDEF1234567 -l \n");
	fprintf(stderr,"usage:./build/httpd -v\n");
	fprintf(stderr,"Options:\n");
	fprintf(stderr,"  --config 生成配置文件模板\n");
	fprintf(stderr,"  --daemon 作为服务进程运行在后台\n");
	fprintf(stderr,"  --port 服务器端口\n");
	fprintf(stderr,"  --NULL 运行过程中无终端输出\n");
	fprintf(stderr,"  --fliter 通过 VIN过滤信息\n");
	fprintf(stderr,"  --SN 通过 序列号过滤信息\n");
	fprintf(stderr,"  --log 保存原始数据\n");
	fprintf(stderr,"  --list 生成设备列表文件模板\n");
	fprintf(stderr,"  --version 显示当前版本\n");
	fprintf(stderr,"  --help 显示帮助信息\n");
	exit(1);
}

static void version(void)
{
	fprintf(stderr,"版本:1.0\n  功能:接收上海中心平台数据协议\n"
			"  提供 RSA 加密通信\n"
			"  实现固件配置文件下载\n"
			"  命令行参数配置端口\n"
			"  提供VIN过滤功能\n"
	       );
	fprintf(stderr,"  Build: %s %s \n\n", __DATE__, __TIME__);
	exit(1);
}

#include "DateTime.h"
#include <stdio.h>
static void UTC2file(const uint32_t times, void* const buf, const size_t _size)
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
}

extern void init_daemon(void);
extern int save_log;
extern int relay;

//int relay = 0;

int init_signals(void)
{
	struct sigaction   sa;
#if 0
	for (sig = signals; sig->signo != 0; sig++) {
		ngx_memzero(&sa, sizeof(struct sigaction));

		if (sig->handler) {
			sa.sa_sigaction = sig->handler;
			sa.sa_flags = SA_SIGINFO;

		} else {
			sa.sa_handler = SIG_IGN;
		}

		sigemptyset(&sa.sa_mask);
		if (sigaction(sig->signo, &sa, NULL) == -1) {
#if (NGX_VALGRIND)
			ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
					"sigaction(%s) failed, ignored", sig->signame);
#else
			ngx_log_error(NGX_LOG_EMERG, log, ngx_errno,
					"sigaction(%s) failed", sig->signame);
			return NGX_ERROR;
#endif
		}
	}
#endif
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	return sigaction(SIGPIPE, &sa, NULL);
}

void do_backtrace() { void *array[100]; char **strings; int size, i; size = backtrace(array, 100); strings = backtrace_symbols(array, size); printf("%p\n", strings); for(i = 0; i < size; i++) printf("sigsegv at :%p:%s\n", array[i], strings[i]); free(strings); }
//jmp_buf env[MAX];
void when_sigsegv() 
{ 
	do_backtrace(); 
	//	int i = find_index((long)pthread_self()); 
	//	printf("sigsegv...%d...\n", i); 
	//	siglongjmp(env[i], MAX); 
}

void handler(int arg)
{
	printf("receive SIGCHLD\n");
}
int reset(void)
{
	signal(SIGCHLD,handler); //注册信号回调函数，当信号发生会调用handler
	pid_t pid;
reboot:
	pid = fork();
	if(pid < 0)
	{
		perror("fork fail ");
		exit(1);
	}
	else if(pid == 0) //子进程
	{
#if 0
		while(1)
		{
			//printf("child \n");
			printf("child pid:%d\n", getpid());
			sleep(1);
		}
#endif
		sleep(1);
	}
	else  //父进程
	{
		sleep(3600*3);  // 每 3 小时重启一次服务器
		kill(pid,SIGKILL);//杀死 pid 发送进程的信号,kill 给其他进程发送信号，指定进程号
		printf("child killed\n");
		//printf("father \n");
		printf("father pid:%d | %d\n", getpid(), pid);
		wait(NULL); //等待回收子进程的资源
		//raise(SIGKILL); //杀死自己的信号,函数raise 给自己发送信号
		goto reboot;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int server_sock = -1;
	//u_short port = 4000;
	u_short port = 9910;
	int client_sock = -1;
	struct sockaddr_in client_name;
	socklen_t  client_name_len = sizeof(client_name);
	//pthread_t newthread;
	char daemon=0;
	struct device_list* device;
	char pwd[128] ;
	int null=0;

	int opt;
	struct option longopts[]={
		{"daemon",0,NULL,'d'},   /* 0->hasn't arg   1-> has arg */
		{"config",0,NULL,'c'},   
		{"list",  0,NULL,'L'},   
		{"port",  1,NULL,'p'},
		{"NULL",  1,NULL,'N'},
#if 0 
		{"sslport",1,NULL,'s'},
		{"extent",0,NULL,'e'},  /* extent function -> https */
#endif
		{"filter",1,NULL,'f'},
		{"SN",    1,NULL,'S'},
		{"log",0,NULL,'l'},
		{"help",0,NULL,'h'},
		{"version",0,NULL,'v'},
		{0,0,0,0}};   /* the last must be a zero array */
	while((opt=getopt_long(argc,argv,":cdNp:f:S:lLhv",longopts,NULL))!=-1)
	{
		switch(opt)
		{
			case 'c':
				create_cfg("./upload/OBD.cfg");
				printf("config file: %s\n", "./upload/OBD.cfg");
				exit(1);
				break;
			case 'L':
				create_list("./upload/Device.list");
				//printf("JSON:%d\n", json_list_search("./upload/Device.list", "102905420128", JSON_LIST_UP));
				printf("JSON:%d\n", json_list_search("./upload/Device.list", "102906420278", JSON_LIST_UP, NULL, NULL));
				printf("List file: %s\n", "./upload/Device.list");
				exit(1);
				break;
			case 'd':
				daemon=1;
				break;
			case 'N':
				null = 1;
				break;
			case 'p':
				//strncpy(port,optarg,15);
				//*portp=port;
				port = atoi(optarg);
				//printf("port : %s \n", optarg); fflush(stdout);
				break;
#if 0
			case 's':
				strncpy(sslport,optarg,15);
				*sslp=sslport;
				break;
			case 'e':
				*dossl=1;
				break;
#endif
			case 'f':
				//strncpy(log,optarg,63);
				//*logp=log;
				printf("filter VIN : %s \n", optarg); fflush(stdout);
				set_filter_vin(optarg);
				break;
			case 'S':
				printf("filter SN : %s \n", optarg); fflush(stdout);
				set_filter_sn(optarg);
				break;
			case 'l':
				save_log = 1;
				break;
			case ':':
				fprintf(stderr,"-%c:option needs a value.\n",optopt);
				exit(1);
				break;
			case 'h':
				usage();
				break;
			case 'v':
				version();
				break;
			case '?':
				fprintf(stderr,"unknown option:%c\n",optopt);
				usage();
				break;
		}
	}

	if (optind < argc) {
		printf("non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");
	}
	if(1==daemon) init_daemon();
	if(1==null)
	{
		int i = 0;
		for(i=0;i<NOFILE;++i)
			close(i);
	}
#if 0
	fflush(stdout);
	setvbuf(stdout,NULL,_IONBF,0);
	UTC2file(time(NULL), pwd, sizeof (pwd));
	printf("test stdout\n");
	//freopen("test1.txt","w",stdout); //注: 不要使用这类的代码 stdout = fopen("test1.txt","w");   这样的话输出很诡异的. 最好使用  freopen 这类的函数来替换它.
	freopen(pwd,"w",stdout); //注: 不要使用这类的代码 stdout = fopen("test1.txt","w");   这样的话输出很诡异的. 最好使用  freopen 这类的函数来替换它.
	printf("test file\n");
	//freopen("/dev/tty","w",stdout);
	//printf("test tty\n");
	//#else
	memset(pwd, 0, sizeof(pwd));
	UTC2file(time(NULL), pwd, sizeof (pwd));
	fflush(stdout);
	//setvbuf(stdout,NULL,_IONBF,0);
	//printf("test stdout\n");
	//int save_fd = dup(STDOUT_FILENO); // 保存标准输出 文件描述符 注:这里一定要用 dup 复制一个文件描述符. 不要用 = 就像是Winodws下的句柄.
	dup(STDOUT_FILENO); // 保存标准输出 文件描述符 注:这里一定要用 dup 复制一个文件描述符. 不要用 = 就像是Winodws下的句柄.
	//int fd = open("test1.txt",(O_RDWR | O_CREAT), 0644);
	int fd = open(pwd, (O_RDWR | O_CREAT), 0644);
	dup2(fd,STDOUT_FILENO); // 用我们新打开的文件描述符替换掉 标准输出
	//printf("test file\n");
#endif
	init_signals();
	signal(SIGSEGV, when_sigsegv);
	struct passwd *npwd;
	npwd = getpwuid(getuid());
	printf("当前登陆的用户名为：%s\n", npwd->pw_name);
	//if(1==daemon) init_daemon();
	memset(pwd, 0, sizeof(pwd));
	//char *p = getcwd(pwd, sizeof(pwd));
	getcwd(pwd, sizeof(pwd));
	//printf("pwd:%s   p:%s size:%d  \n", pwd, p, strlen(pwd));
	printf("Working Directory:%s\n", pwd);
	if(1==daemon) reset();

	thread_list_init();
	server_sock = startup(&port);
	printf("\nhttpd running on port %d\n", port);
	if(9910==port) relay = 0;
	else relay = 1;
	printf("\n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n\n");
	fflush(stdout);
	pthread_lock_init();
	//pool_init (1024); 
	//pool_init (128); 
	pool_init (128); 
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

	close(server_sock);

	return(0);
}
