#ifndef SERVICE_H
#define SERVICE_H
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <QList>
#include <arpa/inet.h>

#define MAX_ACTIVE_COUNT 100

/// \brief 服务类。提供客户端服务
class Service
{
public:
    Service();
    ~Service();
    void initService();
    void AddEpoll(int epollfd);
    static void* wait_epoll(void* arg);
    static void* wait_client(void* arg);
    static void* ReplyData(void* arg);
private:
    int m_Epollfd;
    int m_ActiveConnnectCount;
    struct epoll_event m_ActiveErpoll[MAX_ACTIVE_COUNT];
    QList<int> m_AllActiveSockfd;
    int m_sockfd;
};

#endif // SERVICE_H

