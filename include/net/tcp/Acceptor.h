#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

class EventLoop;
class InetAddress;

/**
 * Acceptor运行在mainLoop中
 * TcpServer发现Acceptor有一个新连接，则将此channel分发给一个subLoop
 */
class Acceptor {
public:
    // 接受新连接的回调函数
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    Acceptor(EventLoop *loop, const InetAddress &ListenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb) {
        NewConnectionCallback_ = cb;
    }

    bool listenning() const { return listenning_; }
    void listen();

private:
    void handleRead();

    EventLoop *loop_;                               // Acceptor用的就是用户定义的BaseLoop
    Socket acceptSocket_;                           // 用于接收连接的套接字(监听套接字)
    Channel acceptChannel_;                         // 用于接收连接的通道(监听套接字对应的Channal)
    NewConnectionCallback NewConnectionCallback_;   // 新连接回调函数  TcpServer::newConnection
    bool listenning_;                               // 是否正在监听的标志
    int idleFd_;                                    // 空闲fd, 用于fd资源不够用时, 可以空一个出来作为新建连接conn fd
};

#endif // ACCEPTOR_H