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
#include "agreement/agreement.h"

#define ISspace(x) isspace((int)(x))

#define SERVER_STRING "Server: jdbhttpd/0.1.0\r\n"
#define STDIN   0
#define STDOUT  1
#define STDERR  2

static const struct agreement_ofp* _agree_obd=NULL;
//static uint8_t obd_buf[1024*10];
static uint8_t msg_buf[4096];

#include <stdio.h>
#if 0
uint8_t hex2int(const uint8_t hex)
{
	uint8_t data=0;
	if(('0'<=hex) && (hex<='9')) data = hex - '0'; // uint8_t
	else if(('A'<=hex) && (hex<='F')) data = hex + 10 - 'A'; // uint8_t
	else data = 0xFF; // error
	return data;
}
#endif
#define  LOG_HEX  1
#undef   LOG_HEX

static char hex_buffer[4094];
static char bin_buffer[4094];
#include "agreement/encrypt.h"
int main(int argc, char *argv[])
{
#if 0
	//rsa_test();
	rsa_main();
	//agreement_test();
#else
	FILE* fd = NULL;
	char* filename = argv[1];
	//char buffer[4094];
	long _size=0;
	long _len=0;
	long i=0;
	//char *p = getcwd(hex_buffer , 40);
	//printf("buffer:%s   p:%s size:%d  \n" , hex_buffer , p , strlen(hex_buffer));
	if(argc<2)
	{
		printf("argc < 2!");
		exit(0);
	}
	//printf("argv[0]: %s\n", argv[0]);
	//printf("argv[1]: %s\n", argv[1]);
	printf("filename: %s\n", filename);
#ifdef LOG_HEX
	fd = fopen(filename, "r");
#else
	fd = fopen(filename, "rb");
#endif
	if(NULL==fd)
	{
		printf("file %s not have!\n", filename);
		exit(0);
	}
	{
		fseek(fd, 0, SEEK_END);
		_size = ftell(fd);
		fseek(fd, 0, SEEK_SET);
		printf("_size :%ld \n", _size);  fflush(stdout);
		if(_size<=0)
		{
			fclose(fd);
			printf("file size error!\n");
			return 0;
		}
		// read
		memset(hex_buffer, 0, sizeof (hex_buffer));
		memset(bin_buffer, 0, sizeof (bin_buffer));
		_size = fread(hex_buffer, 1, _size, fd);
		printf("read: %ld | %ld\n", _size, ftell(fd)); fflush(stdout);
		//fwrite(buf, numchars, 1, fd);
		//                                frrite(buffer, _size, 1, fd);
		//fflush(fd);
		fclose(fd);
		if(_size<=0)
		{
			printf("file read error!\n");
			return 0;
		}
		i=0;
		_len=0;
		while(i<_size)
		{
			while(' '==hex_buffer[i]) i++;
			bin_buffer[_len++] = ((hex2int(hex_buffer[i])&0x0F)<<4) | (hex2int(hex_buffer[i+1])&0x0F);
			i+=2;
		}
		printf("data len: %ld\n", _len);
	}
	_agree_obd = create_agree_obd_shanghai();
	_agree_obd->init(0, (const uint8_t*)"IMEI1234567890ABCDEF", 2, "INFO");
	decode_server(_agree_obd, (const uint8_t *)bin_buffer, _len, msg_buf, sizeof(msg_buf), 0, NULL);

#endif
	return 0;
}

