#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<strings.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>

int main(int argc, char** argv)
{
    int clientFd;
    const char* serverIp = "192.168.1.147";
    int serverPort = 9999;
    struct sockaddr_in serverAddr;
    socklen_t addrLen;

    if((clientFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("socket error:%d,%s\n", errno, strerror(errno));
        return 1;
    }

    addrLen = sizeof(serverAddr);
    bzero(&serverAddr, addrLen);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIp);
    printf("start connect server:%s:%d\n", serverIp, serverPort);
    if(connect(clientFd, (struct sockaddr*)&serverAddr, addrLen) < 0)
    {
        printf("connect error:%d,%s\n", errno, strerror(errno));
        close(clientFd);
        return 1;
    }
    printf("client connected server\n", clientFd);

    const int sendBufSize = 1024;
    char sendBuf[sendBufSize] = {"hello"};
    int sendLen;
    int totalSize = 0;
    printf("udp start send data...\n");
    int num = 3;
    while(num--)
    {
        //注意，测试发现sendto无论什么情况下都不会阻塞
        sprintf(sendBuf,"hello:%d", num);
        //if((sendLen = sendto(clientFd, sendBuf, strlen(sendBuf) + 1, 0, (struct sockaddr*)&serverAddr, addrLen)) <= 0)
        if((sendLen = send(clientFd, sendBuf, strlen(sendBuf) + 1, 0)) <= 0)
        {
            printf("send error:%d,%s\n", errno, strerror(errno));
            close(clientFd);
            return 1;
        }
        totalSize += sendLen;
        printf("send-------:%s\n", sendBuf);
        printf("udp send data size:%d,total size:%d,%dk\n", sendLen, totalSize, totalSize/1024);
        usleep(1000000);
    }

    getchar();
    close(clientFd);
    printf("app end.....\n");
    return 0;
}