#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<strings.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>

int main(int argc, char** argv)
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
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if(bind(listenFd, (struct sockaddr*)&serverAddr, addrLen) < 0)
    {
        printf("bind error:%d,%s\n", errno, strerror(errno));
        return 1;
    }
    //listen的第二个参数指定客户端连接队列的最大值，如果值太小，客户端多路并发连接时可能报错
    if(listen(listenFd, 4) < 0)
    {
        printf("listen error:%d,%s\n", errno, strerror(errno));
        return 1;
    }
    printf("wait client connect......\n");
    if((clientFd = accept(listenFd, (struct sockaddr*)&clientAddr, &addrLen)) < 0)
    {
        printf("accept error:%d,%s\n", errno, strerror(errno));
        return 1;
    }
    printf("client connected fd:%d,addr:%s:%d\n", clientFd, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    //获取客户端socket接收缓冲区大小
    int defRcvBufSize = -1;
    socklen_t optlen = sizeof(defRcvBufSize);
    if(getsockopt(clientFd, SOL_SOCKET, SO_RCVBUF, &defRcvBufSize, &optlen) < 0)
    {
         printf("getsockopt SO_RCVBUF error:%d,%s\n", errno, strerror(errno));
         return 1;
    }
    printf("client socket recv buff size is: %d\n", defRcvBufSize);
    //设置客户端socket接收缓冲区大小
    defRcvBufSize = 1024*10;
    if (setsockopt(clientFd, SOL_SOCKET, SO_RCVBUF, &defRcvBufSize, optlen) < 0)
    {
        printf("setsockopt SO_RCVBUF error:%d,%s\n", errno, strerror(errno));
        return 1;
    }
    if(getsockopt(clientFd, SOL_SOCKET, SO_RCVBUF, &defRcvBufSize, &optlen) < 0)
    {
         printf("getsockopt SO_RCVBUF error:%d,%s\n", errno, strerror(errno));
         return 1;
    }
    printf("client socket recv buff set size is: %d\n", defRcvBufSize);
    getchar();
    char recvBuf[1024];
    while(1)
    {
        int recvlen = recv(clientFd, recvBuf, sizeof(recvBuf), 0);
        if(recvlen > 0)
        {
            printf("recv len:%d\n", recvlen);
            continue;
        }
        else if(recvlen == 0)
        {
            printf("client closed error:%d,%s\n", errno, strerror(errno));
            close(clientFd);
            return 1;
        }
        else
        {
            printf("recv error:%d,%s\n", errno, strerror(errno));
            close(clientFd);
            return 1;
        }
    }
    close(listenFd);
    printf("app end.....\n");
    getchar();
    return 0;
}