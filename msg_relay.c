/*
 * 消息转发
 * */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/*
 * return socket
 * */
int relay_init(const char host[], const int port)
{
    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;
    char ch = 'A';

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(host);
    address.sin_port = htons(port);
    len = sizeof(address);
    result = connect(sockfd, (struct sockaddr *)&address, len);

    if (result == -1)
    {
        //perror("oops: client1");
        //exit(1);
        return -1;
    }
    //write(sockfd, &ch, 1);
    //read(sockfd, &ch, 1);
    return sockfd;
}

int relay_exit(const int socket)
{
    printf("Disconnect : %d\n", socket);
    close(socket);
}

int relay_msg(const int socket, const void* data, const uint16_t _dsize)
{
    int _size = 0;
    _size = write(socket, data, _dsize);
    printf("relay msg[%d]: size:%d, send:%d\n", socket, _dsize, _size);
}




