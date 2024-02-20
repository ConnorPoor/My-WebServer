#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "noncopyable.h"
#include "Thread.h"
#include "../logger/Logging.h"

#include <deque>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <assert.h>

class ThreadPool : noncopyable {
public:
    using ThreadFunction = std::function<void()>;

    explicit ThreadPool(const std::string& name = std::string("ThreadPool"));
    ~ThreadPool();

    // 设置线程池回调函数
    void setThreadInitCallback(const ThreadFunction& cb) { threadInitCallback_ = cb; }
    // 设置线程池大小
    void setThreadSize(const int& num) { threadSize_ = num; }
    // 开始
    void start();
    // 停止
    void stop();

    // 获取线程池名
    const std::string& name() const { return name_; }
    // 获取队列大小
    size_t queueSize() const;

    // 添加线程池工作函数
    void add(ThreadFunction ThreadFunction);

private:
    // 判断线程池是否满了
    bool isFull() const;
    // 线程类初始化时的回调函数
    void runInThread();
    // 从任务队列中取出任务
    ThreadFunction take();

    // 互斥锁
    mutable std::mutex mutex_;
    // 条件变量
    // 用于生产者消费者模型
    std::condition_variable notFull_;
    std::condition_variable notEmpty_;
    // 线程池名，用于打印日志
    std::string name_;
    // 线程初始化之前回调函数
    // 主要用于在线程初始化之前调用
    ThreadFunction threadInitCallback_;
    // 动态数组，用下标来索引智能指针，指向每个线程
    std::vector<std::unique_ptr<Thread>> threads_;
    // 线程池队列，用于争抢任务
    std::deque<ThreadFunction> queue_;
    // 判断线程池是否正在工作
    bool running_;
    // 线程池大小
    size_t threadSize_;
};

# endif // THREAD_POOL_H