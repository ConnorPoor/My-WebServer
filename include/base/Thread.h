#ifndef THREAD_H
#define THREAD_H

#include "noncopyable.h"

#include <thread>
#include <functional>
#include <memory>
#include <string>
#include <atomic>

class Thread : noncopyable {
public:
    using ThreadFunc = std::function<void()>;
    explicit Thread(ThreadFunc, const std::string &name = std::string());
    ~Thread();

    void start(); // 开启线程
    void join();  // 等待线程

    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string& name() const { return name_; }

    static int numCreated() { return numCreated_; }

private:
    void setDefaultName();  // 设置线程名

    bool started_;  // 是否启动线程
    bool joined_;   // 是否等待该线程
    std::shared_ptr<std::thread> thread_;   // 智能指针，指向线程类中真正的线程
    pid_t tid_;     // 线程tid
    // Thread::start() 调用的回调函数
    // 在ThreadPool中初始化为runInThread()函数
    // 其实保存的是 EventLoopThread::threadFunc()
    ThreadFunc func_;   
    std::string name_;  // 线程名
    static std::atomic_int32_t numCreated_; // 线程索引
};

#endif // THREAD_H