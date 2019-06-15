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

#include "agreement/agreement.h"
#include "json_list.h"
#include "thread_list.h"

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
	fprintf(stderr,"usage:./build/httpd [-c --config] [-d --daemon] [-p --port] [-f --fliter] [-l --log] [-L --list] [-v --version] [-h --help]\n\n");
	fprintf(stderr,"usage:./build/httpd -h\n");
	fprintf(stderr,"usage:./build/httpd -p 9910 -f VINABCDEF1234567 -l \n");
	fprintf(stderr,"usage:./build/httpd -v\n");
	exit(1);
}

static void version(void)
{
	fprintf(stderr,"版本:1.0\n功能:接收上海中心平台数据协议\n"
			"提供 RSA 加密通信\n"
			"实现固件配置文件下载\n"
			"命令行参数配置端口\n\n"
			"提供VIN过滤功能\n\n"
	       );
	exit(1);
}

extern void init_daemon(void);
extern int save_log;

int main(int argc, char *argv[])
{
	int server_sock = -1;
	//u_short port = 4000;
	u_short port = 9910;
	int client_sock = -1;
	struct sockaddr_in client_name;
	socklen_t  client_name_len = sizeof(client_name);
	pthread_t newthread;
	char daemon=0;

	int opt;
	struct option longopts[]={
		{"daemon",0,NULL,'d'},   /* 0->hasn't arg   1-> has arg */
		{"config",0,NULL,'c'},   
		{"list",  0,NULL,'L'},   
		{"port",  1,NULL,'p'},
#if 0 
		{"sslport",1,NULL,'s'},
		{"extent",0,NULL,'e'},  /* extent function -> https */
#endif
		{"filter",1,NULL,'f'},
		{"log",0,NULL,'l'},
		{"help",0,NULL,'h'},
		{"version",0,NULL,'v'},
		{0,0,0,0}};   /* the last must be a zero array */
	while((opt=getopt_long(argc,argv,":cdp:f:lLhv",longopts,NULL))!=-1)
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
				printf("List file: %s\n", "./upload/Device.list");
				exit(1);
				break;
			case 'd':
				daemon=1;
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
	thread_list_init();
	server_sock = startup(&port);
	printf("\nhttpd running on port %d\n", port);
	printf("\n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n\n");
	while (1)
	{
		client_sock = accept(server_sock,
				(struct sockaddr *)&client_name,
				&client_name_len);
		if (client_sock == -1)
			error_die("accept");
		/* accept_request(&client_sock); */
		if (pthread_create(&newthread , NULL, (void *)accept_request, (void *)(intptr_t)client_sock) != 0)
			perror("pthread_create");
	}

	close(server_sock);

	return(0);
}
