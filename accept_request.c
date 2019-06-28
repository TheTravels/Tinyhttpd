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
#include "trunking.h"

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
int startup(u_short *);
void unimplemented(int);
int read_threads(int sock, char *buf, int size, int *status);


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

static void csend(const int sockfd, const void *buf, const uint16_t len)
{
	send(sockfd, buf, len, 0);
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

extern int relay;
void accept_request(void *arg)
{
	const char viewer[] = "SocketViewer";
	const struct agreement_ofp* _agree_obd=NULL;
	uint8_t msg_buf[4096];
	int client = 0;//(intptr_t)arg;
	char buf[1024];
	size_t numchars;
	size_t numchars2;
	char method[255];
	char filename[255];
	clock_t start, finish;
	double  duration;
	int recv_status=1;
	time_t time2, time1;
	FILE* fd;
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
	int print=0;

	device = (struct device_list*)arg;
	device->save_log = 1;
	//msg_cache = (struct device_data*)container_of(&(device->msg_list), struct device_data, list);
	//msg_cache = (struct device_data*)container_of((device->msg_list.next), struct device_data, list);
	client = device->socket;
	online_thread_add(device);

	int flags = fcntl(client, F_GETFL, 0);        //获取文件的flags值。
	fcntl(client, F_SETFL, flags | O_NONBLOCK);   //设置成非阻塞模式；
	//usleep(1000*100);   // 100ms
	device->relay_fd = -1;
#if 0
	if(relay)
	{
		printf("转发连接建立中 host:%s port:%d\n", relay_host, relay_port);
		for(i=0; i<1000; i++)
		{
			int sock=0;
			printf("第 %ld 次建立连接 ...\n", i);
			sock = relay_init(relay_host, relay_port);
			device->relay_fd = sock;
			if(sock>=0) break;
			usleep(1000*100);   // 100ms delay
		}
		printf("转发连接建立结束 host:%s port:%d fd:%d\n\n", relay_host, relay_port, device->relay_fd);
	}
#endif
	printf("开始接收数据 TCP: %d\n\n", client);
	start = clock();
	time(&time1);
	_agree_obd = create_agree_obd_shanghai();
	_agree_obd->init(0, (const uint8_t*)"IMEI1234567890ABCDEF", 2, "INFO");
	numchars2 = 0;
	numchars = 0;
	//msg_cache->write=0;
	//msg_cache->read=0;
	while(1)
	{
		//numchars = get_line(client, buf, sizeof(buf));
		numchars = read_threads(client, &buf[numchars2], sizeof(buf)-numchars2, &recv_status);
		if(0==numchars) goto next;
		numchars = numchars + numchars2;
		finish = clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		time(&time2);
		duration = difftime(time2, time1);
		//printf( "recv time: %f seconds\n\n", duration );
		//printf( "recv time: tart:%ld  finish:%ld duration:%f seconds\n\n", start, finish, duration );
		if(print) printf( "\nrecv time: time1:%ld  time2:%ld duration:%f seconds\n", time1, time2, duration );
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

		if(0==memcmp(buf, viewer, sizeof(viewer)))
		{
			printf("view: %s\n", viewer);
			numchars2 = 0;
			if(device->relay_fd>=0) relay_exit(device->relay_fd);
			device->relay_fd = -1;
			if(NULL!=device) online_thread_free(device);
			trunking(client);
			goto next;
		}
		else if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
		{
			if(relay && (-1==device->relay_fd))
			{
				printf("转发连接建立中 host:%s port:%d\n", relay_host, relay_port);
				for(i=0; i<1000; i++)
				{
					int sock=0;
					printf("第 %ld 次建立连接 ...\n", i);
					sock = relay_init(relay_host, relay_port);
					device->relay_fd = sock;
					if(sock>=0) break;
					usleep(1000*100);   // 100ms delay
				}
				printf("转发连接建立结束 host:%s port:%d fd:%d\n\n", relay_host, relay_port, device->relay_fd);
			}
#if 0
			printf("TCP/IP connect: %s\n", buf);
			send(client, "ACK", 3, 0);
			unimplemented(client);
#endif
			timer = time(NULL);
			memset(filename, 0, sizeof(filename));
			UTC2file(timer, filename, sizeof(filename));
			if(print) printf("TCP/IP connect[%d]: %s SN:%s\n", (int)numchars, buf, device->sn);
			//printf("TCP/IP connect[%d]: %s SN:%s\n", (int)numchars, buf, device->sn);
			//if(0!=decode_server(&print, _agree_obd, (const uint8_t *)buf, numchars, msg_buf, sizeof(msg_buf), client, csend))
			if(0!=decode_server(&print, _agree_obd, (const uint8_t *)buf, numchars, msg_buf, sizeof(msg_buf), device, csend))
				//if(0!=decode_server(&print, _agree_obd, (const uint8_t *)buf, numchars, msg_cache->data, DEVICE_DATA_SIZE, device, csend))
			{
				numchars2 = numchars;
				goto next;
			}
			//msg_cache->write++;
			//if(msg_cache->write>=DEVICE_ITEM_SIZE) msg_cache->write = 0;
			numchars2 = 0;
			if(save_log)
			{
				fd = NULL;
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
				numchars = get_line(client, buf, sizeof(buf));
			not_found(client);
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
				serve_file(client, path);
			else
				execute_cgi(client, path, method, query_string);
		}
next:
		usleep(1000*10);   // 10ms delay
		if(0==recv_status) // close
		{
			break;
		}
	}
	printf("\n连接断开\n");

	if(device->relay_fd>=0) relay_exit(device->relay_fd);
	close(client);
	if(NULL!=device) online_thread_free(device);
	pthread_exit(NULL);
}

/**********************************************************************/
/* Inform the client that a request it has made has a problem.
 * Parameters: client socket */
/**********************************************************************/
void bad_request(int client)
{
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
		send(client, buf, strlen(buf), 0);
		fgets(buf, sizeof(buf), resource);
	}
}

/**********************************************************************/
/* Inform the client that a CGI script could not be executed.
 * Parameter: the client socket descriptor. */
/**********************************************************************/
void cannot_execute(int client)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
	send(client, buf, strlen(buf), 0);
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
int get_line(int sock, char *buf, int size)
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
}

/**********************************************************************/
/* Give a client a 404 not found status message. */
/**********************************************************************/
void not_found(int client)
{
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
}


