#include "TcpServer.h"
#include "Logging.h"
#include "AsyncLogging.h"
#include "EventLoop.h"

class EchoServer {
public:
    EchoServer(EventLoop* loop, const InetAddress& addr, const std::string& name) :
    loop_(loop), server_(loop_, addr, name) {
        server_.setThreadNum(4);
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void start() {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        if (conn->connected()) {
            LOG_INFO << "Connection UP : " << conn->peerAddress().toIpPort().c_str();
        } else {
            LOG_INFO << "Connection DOWN : " << conn->peerAddress().toIpPort().c_str();
        }
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
        std::string msg = buf->retrieveAllAsString();
        LOG_INFO << conn->name() << " recieved " << msg.size() << " at " << time.toFormattedString();
        conn->send(msg);
    }

    EventLoop* loop_;
    TcpServer server_;
};

const int RollSize = 1000 * 1000 * 500;
std::unique_ptr<AsyncLogging> async_log;

void asyncLogout(const char* msg, int len) {
    async_log->append(msg, len);
}

void settingAsyncLog(const char* argv0) {
    Logger::setOutput(asyncLogout);
    async_log.reset(new AsyncLogging(argv0, RollSize));
    async_log->start();
}

int main(int argc, char* argv[]) {
    settingAsyncLog(argv[0]);
    EventLoop loop;
    InetAddress addr(8088);
    EchoServer server(&loop, addr, "EchoServer");
    server.start();
    loop.loop();
}