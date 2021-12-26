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
    int serverPort = 8888;
    struct sockaddr_in serverAddr, clientAddr;
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
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if(bind(clientFd, (struct sockaddr*)&serverAddr, addrLen) < 0)
    {
        printf("connect error:%d,%s\n", errno, strerror(errno));
        close(clientFd);
        return 1;
    }
    printf("udp client binded port:%d\n", serverPort);

    const int sendBufSize = 1024;
    char sendBuf[sendBufSize] = {0};
    int recvLen;
    int totalSize = 0;
    printf("udp start recv data...\n");
    while(1)
    {
        //这里如果对方关闭了socket一段时间(如果是同步send只有几秒，异步send，时间要久点)后，再send时会收到SIGPIPE信号
        if((recvLen = recvfrom(clientFd, sendBuf, sendBufSize, 0, (struct sockaddr*)&clientAddr, &addrLen)) <= 0)
        {
            printf("recvfrom error:%d,%s\n", errno, strerror(errno));
            close(clientFd);
            return 1;
        }
        printf("udp recvfrom:%s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        totalSize += recvLen;
        printf("udp recv data size:%d,total size:%d,%dk\n", recvLen, totalSize, totalSize/1024);
        sleep(3);
    }

    getchar();
    close(clientFd);
    printf("app end.....\n");
    return 0;
}