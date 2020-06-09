#include "service.h"

#define INVALID -1

Service::Service():m_Epollfd(INVALID), m_ActiveConnnectCount(INVALID)
  , m_sockfd(INVALID)
{
    m_Epollfd = epoll_create1(EPOLL_CLOEXEC);
    if(m_Epollfd == INVALID) {
        perror("epoll_create1");
        return;
    }
    initService();
}

Service::~Service()
{
    close(m_Epollfd);
    for (int &connectFd:m_AllActiveSockfd) {
        close(connectFd);
    }
    close(m_sockfd);
}

void Service::initService()
{
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_sockfd == INVALID) {
        perror("socket");
        return;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(7007);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int nbind = bind(m_sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if(nbind == -1) {
        perror("bind");
        return;
    }
    listen(m_sockfd, 5);
    pthread_t tid;
    pthread_create(&tid, NULL, wait_client, this);
    pthread_join(tid, NULL);
}

void Service::AddEpoll(int connectFd)
{
    struct epoll_event event;
    event.data.fd = connectFd;
    event.events = EPOLLET|EPOLLIN;
    int epollCtl = epoll_ctl(m_Epollfd, EPOLL_CTL_ADD, connectFd, &event);
    if(epollCtl == INVALID) {
        perror("epoll_ctl");
        return;
    }
}

void *Service::wait_epoll(void *arg)
{
    Service* ser = (Service*)arg;
    while(1) {
        int count = epoll_wait(ser->m_Epollfd, ser->m_ActiveErpoll, 100, -1);
        if(count == INVALID) {
            perror("epoll_wait");
            return NULL;
        }
        else {
            pthread_t tid;
            pthread_create(&tid, NULL, ser->ReplyData, arg);
        }
    }
    return NULL;
}

void *Service::wait_client(void *arg)
{
    Service* ser = (Service*)arg;
    struct sockaddr_in addr_in;
    socklen_t size = sizeof(addr_in);
    pthread_t tid;
    pthread_create(&tid, NULL, wait_epoll, arg);
    while(1) {
        int connectFd = accept(ser->m_sockfd, (struct sockaddr*)&addr_in, &size);
        if(connectFd == INVALID) {
            perror("accept");
            return NULL;
        }
        else {
            ser->m_AllActiveSockfd.append(connectFd);
            ser->AddEpoll(connectFd);
        }
    }
    return NULL;
}

void *Service::ReplyData(void *arg)
{
    char buf[128] = {0};
    Service* ser = (Service*)arg;
    for (int i = 0;i < ser->m_ActiveConnnectCount;i++) {
        int size = read(ser->m_ActiveErpoll[i].data.fd, buf, 128);
        if(size <= 0) {
            epoll_ctl(ser->m_Epollfd, EPOLL_CTL_DEL, ser->m_ActiveErpoll[i].data.fd, NULL);
            ser->m_AllActiveSockfd.removeOne(ser->m_ActiveErpoll[i].data.fd);
            close(ser->m_ActiveErpoll[i].data.fd);
            return NULL;
        }
        for (int &connectFd: ser->m_AllActiveSockfd) {
            write(1, buf, size);
            write(connectFd, buf, size);
        }
    }
    return NULL;
}
