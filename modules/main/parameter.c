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
#include <getopt.h>
#include <pwd.h>
#include <signal.h>
#include <sys/param.h>
#include <syslog.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
//#include "../obd/thread_vin.h"
#include "../obd/json_list.h"
//#include "modules/config/config_data.h"

void init_daemon(void)
{
    //int i;
    pid_t pid;
    struct sigaction sa;
    umask(0);
    pid=fork();
    if(pid>0)
        exit(0);
    else if(pid<0)
        exit(1);
    setsid();

    sa.sa_handler=SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;
    sigaction(SIGHUP,&sa,NULL);

#if 1
    pid=fork();
    if(pid>0)
        exit(0);
    else if(pid<0)
        exit(1);
#if 0
    for(int i=0;i<NOFILE;++i)
        close(i);
#endif
#endif
    //chdir("/");
    //chdir("/home/public/server");
    //chdir(_daemon_path);
}

void handler(int arg)
{
    printf("receive SIGCHLD: %d \n", arg);
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
        //wait(NULL); //等待回收子进程的资源
        waitpid(pid, NULL, 0); //等待回收子进程的资源
        //raise(SIGKILL); //杀死自己的信号,函数raise 给自己发送信号
        goto reboot;
    }
    return 0;
}

static void usage(void)
{
	//fprintf(stderr,"usage:./main [-d --daemon] [-p --port] [-s --sslport] [-l --log] [-v --version] [-h --help]\n\n");
	fprintf(stderr,"usage:./build/httpd [-c --config] [-d --daemon] [-D --Dir] [-p --port] [-N --NULL] [-f --fliter] [-S --SN] [-l --log] [-L --list] [-v --version] [-h --help]\n\n");
	fprintf(stderr,"usage:./build/httpd -h\n");
	fprintf(stderr,"usage:./build/httpd -p 9910 -f VINABCDEF1234567 -l \n");
	fprintf(stderr,"usage:./build/httpd -v\n");
	fprintf(stderr,"Options:\n");
	fprintf(stderr,"  --config 生成配置文件模板\n");
	fprintf(stderr,"  --daemon 作为服务进程运行在后台\n");
	fprintf(stderr,"  --Dir 指定工作目录, 默认为当前目录\n");
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
	fprintf(stderr,"版本:2.0\n  功能:接收上海中心平台数据协议\n"
			"  支持广州云景协议\n"
			"  提供 RSA 加密通信\n"
			"  实现固件配置文件下载\n"
			"  命令行参数配置端口\n"
			"  提供VIN过滤功能\n"
	       );
	fprintf(stderr,"  Build: %s %s \n\n", __DATE__, __TIME__);
	exit(1);
}

extern int save_log;

int cmd_parameter(int argc, char *argv[])
{
	//u_short port = 4000;
    int port = 9910;
	char daemon=0;
	int null=0;
    //char _daemon_path[128] = "~/tools/Tinyhttpd";
    char _daemon_path[128] = "./";
    //char _daemon_path[128] = "/home/public/server";

	int opt;
	struct option longopts[]={
		{"daemon",0,NULL,'d'},   /* 0->hasn't arg   1-> has arg */
		{"Dir"   ,0,NULL,'D'},   /* 0->hasn't arg   1-> has arg */
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
	struct passwd *npwd;
	npwd = getpwuid(getuid());
	//printf("当前登陆的用户名为：%s\n", npwd->pw_name);
	memset(_daemon_path, 0, sizeof(_daemon_path));
	snprintf(_daemon_path, sizeof(_daemon_path)-1, "/home/%s/tools/Tinyhttpd", npwd->pw_name);
	while((opt=getopt_long(argc,argv,":cdD:Np:f:S:lLhv",longopts,NULL))!=-1)
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
                //set_filter_vin(optarg);
                usage();
				break;
			case 'S':
				printf("filter SN : %s \n", optarg); fflush(stdout);
                //set_filter_sn(optarg);
                usage();
                break;
			case 'D':
				printf("Work Dir: %s \n", optarg); fflush(stdout);
				if(strlen(optarg)<sizeof(_daemon_path)) 
				{
					memset(_daemon_path, 0, sizeof(_daemon_path));
					memcpy(_daemon_path, optarg, strlen(optarg));
				}
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
        usage();
	}
    if(1==daemon)
    {
        init_daemon();
        reset();
    }
	chdir(_daemon_path);
	if(1==null)
	{
		int i = 0;
		for(i=0;i<NOFILE;++i)
			close(i);
	}

    return port;
}
