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
    const char* serverIp = "192.168.2.162";
    int serverPort = 8888;
    struct sockaddr_in serverAddr;
    socklen_t addrLen;

    if((clientFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
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
    //signal(SIGPIPE, SIG_IGN);
    sleep(5);
    const int sendBufSize = 1024;
    char sendBuf[sendBufSize];
    int sendLen;
    int totalSize = 0;
    while(1)
    {
        printf("start send data...\n");
        //这里如果对方关闭了socket一段时间(如果是同步send只有几秒，异步send，时间要久点)后，再send时会收到SIGPIPE信号
        if((sendLen = send(clientFd, sendBuf, sendBufSize, 0)) < 0)
        {
            printf("send error:%d,%s\n", errno, strerror(errno));
            close(clientFd);
            return 1;
        }
        totalSize += sendLen;
        printf("send data size:%d,total size:%d,%dk\n", sendLen, totalSize, totalSize/1024);
    }

    getchar();
    close(clientFd);
    printf("app end.....\n");
    return 0;
}