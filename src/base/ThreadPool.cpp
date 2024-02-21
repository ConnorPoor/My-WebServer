#include "ThreadPool.h"

#include <iostream>

ThreadPool::ThreadPool(const std::string& name) :
    mutex_(),       // 初始化互斥量
    notFull_(),     // 初始化条件变量
    notEmpty_(),    // 初始化条件变量
    name_(name),    // 初始化线程池名
    running_(false) // 默认未开始工作
{}

ThreadPool::~ThreadPool() {
    // 先停止线程池工作
    stop();
    for (const auto& t : threads_) {
        // 分离每个线程（此处调用的join()函数为线程类的函数，其内部调用真正的join()函数）
        t->join();
    }
}

// 线程池开始工作
void ThreadPool::start() {
    // 将运行状态改为true
    running_ = true;
    // 初始化线程数组
    threads_.reserve(threadSize_);
    for (int i = 0; i < threadSize_; ++i) {
        char id[32];
        snprintf(id, sizeof(id), "%d", i + 1);
        // 构造插入，减少拷贝次数，将ThreadPool中的runInThread()函数作为工作函数传给Thread类对象
        // 第二个参数是this指针，因为这是一个成员函数，调用的第一个参数是一个隐含的this指针
        // 线程池中对象的名字为线程池名+id编号
        threads_.emplace_back(new Thread(
            std::bind(&ThreadPool::runInThread, this), name_ + id));
        threads_[i]->start();
    }
    // 不创建新线程
    if (threadSize_ == 0 && threadInitCallback_) {
        threadInitCallback_();
    }
}

// 停止线程池函数
// 通过lock_guard构造时自动上锁，析构时释放锁来处理共享数据running_
// 在start()中不用上锁是因为没有操作共享数据
void ThreadPool::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    running_ = false;
    notEmpty_.notify_all(); // 唤醒所有线程
    notFull_.notify_all(); // 唤醒所有线程
}

// 获取线程池线程组的大小
size_t ThreadPool::queueSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}
// 判断线程池任务队列是否满
bool ThreadPool::isFull() const {
    return threadSize_ > 0 && queue_.size() >= threadSize_;
}

// 线程池添加任务，生产者
void ThreadPool::add(ThreadFunction ThreadFunction) {
    // 当子线程数量为0
    if (threads_.empty()) {
        ThreadFunction();
    } else{
        std::unique_lock<std::mutex> lock(mutex_);
        while (isFull() && running_) {
            notFull_.wait(lock);
        }
        if (!running_) return;
        assert(!isFull());

        queue_.push_back(std::move(ThreadFunction));
        notEmpty_.notify_one();
    }
}

// 从任务队列中取出任务，消费者
ThreadPool::ThreadFunction ThreadPool::take() {

    std::unique_lock<std::mutex> lock(mutex_);

    while (queue_.empty() && running_) {
        notEmpty_.wait(lock);
    }
    ThreadFunction task;
    if (!queue_.empty()) {
        task = queue_.front();
        queue_.pop_front();
        if (threadSize_ > 0) {
            notFull_.notify_one();
        }
    }
    return task;
}

// 线程内执行的回调函数
void ThreadPool::runInThread() {
    try {
        // 执行某些线程进入前的初始化工作
        if (threadInitCallback_) {
            threadInitCallback_();
        }
        // 线程循环等待任务
        while (running_) {
            ThreadFunction task(take());
            if (task) {
                task();
            }
        }
    } catch(...) {
        // LOG_WARN << "runInThread throw exception";
    }
}
