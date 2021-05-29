//set time out connect
//没找到让connect超时的办法
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <iostream>

int timeout_connect(const char *ip, int port, int time)
{
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    struct timeval timeout;
    timeout.tv_sec = time;
    timeout.tv_usec = 0;
    socklen_t len = sizeof(timeout);
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, len);
    assert(ret != -1);

    ret = connect(sockfd, (struct sockaddr *)&address, sizeof(address));
    std::cout << ret;
    if (ret == -1)
    {
        std::cout << errno << std::endl;
        if (errno == EINPROGRESS)
        {
            printf("connecting time out, process timeout logic\n");
            return -1;
        }
        printf("ERROR!\n");
        return -1;
    }
    return sockfd;
}

int main(int argc, char *argv[])
{
    assert(argc > 2);
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = timeout_connect(ip, port, 10);
    std::cout << sockfd << std::endl;
    if (sockfd < 0)
    {
        return 1;
    }
    return 0;
}