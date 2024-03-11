#ifndef HTTP_HTTPSERVER_H
#define HTTP_HTTPSERVER_H

#include "TcpServer.h"
#include "noncopyable.h"
#include "Logging.h"
#include <string>

class HttpRequest;
class HttpResponse;

class HttpServer : noncopyable {
public:
    // HTTP回调函数
    using HttpCallback = std::function<void (const HttpRequest&, HttpResponse*)>;

    HttpServer(EventLoop *loop,
            const InetAddress& listenAddr,
            const std::string& name,
            TcpServer::Option option = TcpServer::kNoReusePort);
    
    // 获得主线程TCP服务器对应的EventLoop
    EventLoop* getLoop() const { return server_.getLoop(); }

    // 设置HTTP回调函数
    void setHttpCallback(const HttpCallback& cb) {
        httpCallback_ = cb;
    }
    
    // 启动服务器
    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime);
    void onRequest(const TcpConnectionPtr&, const HttpRequest&);

    // 封装了TCP服务器
    TcpServer server_;
    // HTTP回调函数
    HttpCallback httpCallback_;
};

#endif // HTTP_HTTPSERVER_H