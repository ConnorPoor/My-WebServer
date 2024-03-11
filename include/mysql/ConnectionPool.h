#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include "MysqlConn.h"
#include "json.hpp"
using json = nlohmann::json; 

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>

// 单例模式
class ConnectionPool {
public:
    static ConnectionPool* getConnectionPool(std::string filePath);
    std::shared_ptr<MysqlConn> getConnection();
    ~ConnectionPool();

private:
    // 私有构造函数防止构造对象
    ConnectionPool(std::string filePath);
    // 禁止拷贝构造
    ConnectionPool(const ConnectionPool& obj) = delete;
    // 禁止移动构造
    ConnectionPool(const ConnectionPool&& obj) = delete;
    // 禁止赋值构造
    ConnectionPool& operator=(const ConnectionPool& obj) = delete;

    // 解析JSON配置文件
    bool parseJsonFile();
    // 制造连接线程函数
    void produceConnection();
    // 销毁过久空闲连接函数
    void recycleConnection();
    // 添加数据库连接
    void addConnection();

    // TODO:加上文件路径
    // std::string filePath_;
    std::string filePath_;
    std::string ip_;
    std::string user_;
    std::string passwd_;
    std::string dbName_;
    unsigned short port_;
    int minSize_;
    int maxSize_;
    int currentSize_;
    int timeout_;
    int maxIdleTime_;
    std::queue<MysqlConn*> connectionQueue_; // 连接池队列
    std::mutex mutex_; 
    std::condition_variable cond_;
};

#endif // CONNECTION_POOL_H