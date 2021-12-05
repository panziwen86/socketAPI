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
#include<fcntl.h>

int main(int argc, char** argv)
{
    int clientFd;
    const char* serverIp = "192.168.2.147";
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
    //设置为异步socket
    int opts = fcntl(clientFd, F_GETFL);
    fcntl(clientFd, F_SETFL, opts | O_NONBLOCK);
    //不知道干嘛
    linger lger = {1,0};
    setsockopt(clientFd, 0, SO_LINGER, (const char*)&lger, sizeof(linger));
    //signal(SIGPIPE, SIG_IGN);
    const int sendBufSize = 1024;
    char sendBuf[sendBufSize];
    int sendLen;
    int totalSize = 0;
    sleep(20);
    while(1)
    {
        struct timeval timeout;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(clientFd, &wfds);
        int maxfd = clientFd+1;
        int ret = select(maxfd, NULL, &wfds, NULL, &timeout);
        if(ret == 0)
        {
            printf("no sendbuf size wait...\n");
            continue;
        }
        else if(ret > 0)
        {
            if(FD_ISSET(clientFd, &wfds))
            {
                printf("start send data...\n");
                //这里如果对方关闭了socket一段时间(如果是同步send只有几秒，异步send，时间要久点)后，再send时会收到SIGPIPE信号
                sendLen = send(clientFd, sendBuf, sendBufSize, 0);
                if(sendLen < 0)
                {
                    printf("send error:%d,%s\n", errno, strerror(errno));
                    close(clientFd);
                    return 1;
                }
                totalSize += sendLen;
                printf("send data size:%d,total size:%d,%dk\n", sendLen, totalSize, totalSize/1024);
            }
        }
        else
        {
            printf("select error:%d,%s\n", errno, strerror(errno));
            close(clientFd);
            return 1;
        }
    }

    getchar();
    close(clientFd);
    printf("app end......\n");
    return 0;
}