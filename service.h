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
#include <QThreadPool>
#include <QDebug>

#define MAX_ACTIVE_COUNT 100 ///< 同时活跃的最大总数量
/// \brief 线程运行函数声明
class runInstance;

/// \brief 服务类。提供客户端服务
class Service
{
public:
    /// \brief 构造函数
    Service();

    /// \brief 析构函数
    ~Service();

    /// \brief 初始化服务器的配置信息
    void initService();

    /// \brief 添加epoll
    /// \param[in] epollfd 客户端已经上线的描述符
    void AddEpoll(int epollfd);

    /// \brief 解析协议
    /// \param[in] content 协议内容
    /// \param[in] fd 文件描述符
    void AnalysisProtocol(char* content, int fd);

    /// \brief 阻塞等待epoll输入事件
    /// \param[in] arg 线程函数参数
    /// \retval 返回线程函数的返回值
    static void* wait_epoll(void* arg);

    /// \brief 阻塞等待客户端到来
    /// \param[in] arg 线程函数参数
    /// \retval 返回线程函数的返回值
    static void* wait_client(void* arg);

private:
    int m_Epollfd;  ///< epoll描述符
    int m_ActiveConnnectCount;   ///< 当前活跃数量的值
    struct epoll_event m_ActiveErpoll[MAX_ACTIVE_COUNT];  ///< 活跃的epoll信息
    QList<int> m_AllActiveSockfd;   ///< 所有已经上线的客户端套接字
    int m_sockfd; ///< 套接字
    QThreadPool m_threadPool;  ///< 线程池类
    QMap<std::string, int> m_mapUserNUm_Fd; ///< 用户号码和套接字描述符
    friend class runInstance;  ///< 线程运行函数，可访问私有成员
};

/// \brief 线程池运行类，处理业务逻辑
class runInstance : public QRunnable {
public:
    /// \brief 构造函数
    /// \param[in] target 目标信息
    runInstance(Service* target);

    /// \brief 线程运行函数
    void run();

private:
    Service* m_target;  ///< 目标服务信息
};

#endif // SERVICE_H

