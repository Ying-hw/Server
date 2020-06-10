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
    m_threadPool.setExpiryTimeout(INVALID);
    m_threadPool.setMaxThreadCount(MAX_ACTIVE_COUNT);
    initService();
}

Service::~Service()
{
    m_threadPool.clear();
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
    if(nbind == INVALID) {
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
        ser->m_ActiveConnnectCount = epoll_wait(ser->m_Epollfd, ser->m_ActiveErpoll, MAX_ACTIVE_COUNT, INVALID);
        if(ser->m_ActiveConnnectCount == INVALID) {
            perror("epoll_wait");
            return NULL;
        }
        else {
            qDebug("开始运行线程");
            runInstance* run = new runInstance(ser);
            ser->m_threadPool.start(run);
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

runInstance::runInstance(Service *target) : m_target(target)
{
    setAutoDelete(true);
}

void runInstance::run()
{
    char buf[128] = {0};
    qDebug() << "size:" << m_target->m_ActiveConnnectCount;
    for (int i = 0;i < m_target->m_ActiveConnnectCount;i++) {
        int size = read(m_target->m_ActiveErpoll[i].data.fd, buf, 128);
        if(size <= 0) {
            epoll_ctl(m_target->m_Epollfd, EPOLL_CTL_DEL, m_target->m_ActiveErpoll[i].data.fd, NULL);
            m_target->m_AllActiveSockfd.removeOne(m_target->m_ActiveErpoll[i].data.fd);
            close(m_target->m_ActiveErpoll[i].data.fd);
            qDebug("已经退出");
            return;
        }
        for (int &connectFd: m_target->m_AllActiveSockfd) {
            write(1, buf, size);
            write(connectFd, buf, size);
        }
    }
}
