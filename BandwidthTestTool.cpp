#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<strings.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include"common.h"
#include<string>
#include<stdlib.h>

int TcpClient(char* serverIp, unsigned short serverPort, bool bSend);
int TcpServer(unsigned short port, bool bSend);

int main(int argc, char** argv)
{
    std::string type;
    if(argc > 1)
    {
        type = argv[1];
    }
    if(argc < 3 || (type != "ss" && type != "sr" && type != "cs" && type != "cr") 
        || ((type == "cs" || type == "cr") && argc < 4))
    {
        printf("Usage:\n"
                    "Server send data: %s ss listenPort\n"
                    "Server recv data: %s sr listenPort\n"
                    "Client send data: %s cs serverIp serverPort\n"
                    "Client recv data: %s cr serverIp serverPort\n", argv[0], argv[0], argv[0], argv[0]);
        return 1;
    }
    if(type == "sr")
    {
        TcpServer(atoi(argv[2]), false);
    }
    else if(type == "cs")
    {
        TcpClient(argv[2], atoi(argv[3]), true);
    }
    else if(type == "ss")
    {
        TcpServer(atoi(argv[2]), true);
    }
    else
    {
        TcpClient(argv[2], atoi(argv[3]), false);
    }

    return 0;
}

int TcpClient(char* serverIp, unsigned short serverPort, bool bSend)
{
    int clientFd;
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
    int setBufSize = 1024*20;
    socklen_t optlen = sizeof(setBufSize);
    const int dataBufSize = 1024*10;
    char dataBuf[dataBufSize];
    int dataLen;
    long long totalSize = 0;
    long long startTime = GetTick();
    long long lastPrintTime = startTime;
    if(bSend)
    {
        if (setsockopt(clientFd, SOL_SOCKET, SO_SNDBUF, &setBufSize, optlen) < 0)
        {
            printf("setsockopt SO_SNDBUF error:%d,%s\n", errno, strerror(errno));
            return 1;
        }
        printf("start send data...\n");
        while(1)
        {
            //这里如果对方关闭了socket一段时间(如果是同步send只有几秒，异步send，时间要久点)后，再send时会收到SIGPIPE信号
            if((dataLen = send(clientFd, dataBuf, dataBufSize, 0)) < 0)
            {
                printf("send error:%d,%s\n", errno, strerror(errno));
                break;
            }
            totalSize += dataLen;
            long long curTime = GetTick();
            if(curTime - lastPrintTime >= 3*1000)
            {
                double times = (curTime - startTime)/1000.0;
                double speedbps = (totalSize * 8.0) / times;
                printf("send data total size:%dk,speed:%.2fKbps,%.2fMbps\n", totalSize/1024, speedbps/1024, speedbps/1024/1024);
                lastPrintTime = curTime;
            }
            usleep(1000);
        }
    }
    else
    {
        if (setsockopt(clientFd, SOL_SOCKET, SO_RCVBUF, &setBufSize, optlen) < 0)
        {
            printf("setsockopt SO_RCVBUF error:%d,%s\n", errno, strerror(errno));
            return 1;
        }
        printf("start recv data...\n");
        while(1)
        {
            if((dataLen = recv(clientFd, dataBuf, sizeof(dataBufSize), 0)) <= 0)
            {
                printf("recv error:%d,%s\n", errno, strerror(errno));
                break;
            }
            totalSize += dataLen;
            long long curTime = GetTick();
            if(curTime - lastPrintTime >= 3*1000)
            {
                double times = (curTime - startTime)/1000.0;
                double speedbps = (totalSize * 8.0) / times;
                printf("recv data total size:%dk,speed: %.2fKbps,%.2fMbps\n", totalSize/1024, speedbps/1024, speedbps/1024/1024);
                lastPrintTime = curTime;
            }
        }
    }
    close(clientFd);
    return 0;
}

int TcpServer(unsigned short port, bool bSend)
{
    int listenFd;
    int clientFd;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen;

    if((listenFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket error:%d,%s\n", errno, strerror(errno));
        return 1;
    }
    //设置端口可重复绑定，否则，如果之前这个端口有未完成的socket，或者多个进程同时bind这个端口，会报错，还有一个SO_REUSEADDR参数暂不了解
    int reuse_port = 1;
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port)) < 0)
    {
        printf("setsockopt SO_REUSEADDR error:%d\n", errno);
        return 1;
    }
    addrLen = sizeof(serverAddr);
    bzero(&serverAddr, addrLen);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if(bind(listenFd, (struct sockaddr*)&serverAddr, addrLen) < 0)
    {
        printf("bind error:%d,%s\n", errno, strerror(errno));
        return 1;
    }
    //listen的第二个参数指定客户端连接队列的最大值，如果值太小，客户端多路并发连接时可能报错
    if(listen(listenFd, 1) < 0)
    {
        printf("listen error:%d,%s\n", errno, strerror(errno));
        return 1;
    }
    printf("server start listen port:%u\n", port);
    while(1)
    {
        printf("wait client connect......\n");
        if((clientFd = accept(listenFd, (struct sockaddr*)&clientAddr, &addrLen)) < 0)
        {
            printf("accept error:%d,%s\n", errno, strerror(errno));
            return 1;
        }
        printf("client connected fd:%d,addr:%s:%d\n", clientFd, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        //设置客户端socket接收缓冲区大小
        int setBufSize = 1024*20;
        socklen_t optlen = sizeof(setBufSize);
        if(bSend)
        {
            if (setsockopt(clientFd, SOL_SOCKET, SO_SNDBUF, &setBufSize, optlen) < 0)
            {
                printf("setsockopt SO_SNDBUF error:%d,%s\n", errno, strerror(errno));
                close(clientFd);
                continue;
            }
            printf("start send data...\n");
        }
        else
        {
            if (setsockopt(clientFd, SOL_SOCKET, SO_RCVBUF, &setBufSize, optlen) < 0)
            {
                printf("setsockopt SO_RCVBUF error:%d,%s\n", errno, strerror(errno));
                close(clientFd);
                continue;
            }
            printf("start recv data...\n");
        }
        const int dataBufSize = 1024*10;
        char dataBuf[dataBufSize];
        int dataLen;
        long long totalSize = 0;
        long long startTime = GetTick();
        long long lastPrintTime = startTime;
        if(bSend)
        {
            while(1)
            {
                //这里如果对方关闭了socket一段时间(如果是同步send只有几秒，异步send，时间要久点)后，再send时会收到SIGPIPE信号
                if((dataLen = send(clientFd, dataBuf, dataBufSize, 0)) < 0)
                {
                    printf("send error:%d,%s\n", errno, strerror(errno));
                    close(clientFd);
                    break;
                }
                totalSize += dataLen;
                long long curTime = GetTick();
                if(curTime - lastPrintTime >= 3*1000)
                {
                    double times = (curTime - startTime)/1000.0;
                    double speedbps = (totalSize * 8.0) / times;
                    printf("send data total size:%dk,speed:%.2fKbps,%.2fMbps\n", totalSize/1024, speedbps/1024, speedbps/1024/1024);
                    lastPrintTime = curTime;
                }
                usleep(1000);
            }
        }
        else
        {
            while(1)
            {
                if((dataLen = recv(clientFd, dataBuf, sizeof(dataBufSize), 0)) <= 0)
                {
                    printf("recv error:%d,%s\n", errno, strerror(errno));
                    close(clientFd);
                    break;
                }
                totalSize += dataLen;
                long long curTime = GetTick();
                if(curTime - lastPrintTime >= 3*1000)
                {
                    double times = (curTime - startTime)/1000.0;
                    double speedbps = (totalSize * 8.0) / times;
                    printf("recv data total size:%dk,speed: %.2fKbps,%.2fMbps\n", totalSize/1024, speedbps/1024, speedbps/1024/1024);
                    lastPrintTime = curTime;
                }
            }
        }
    }
    close(listenFd);

    return 0;
}