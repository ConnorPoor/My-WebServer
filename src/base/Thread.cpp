#include <semaphore.h>
#include "../../include/base/Thread.h"
#include "../../include/base/CurrentThread.h"

// 初始化线程索引为0
std::atomic_int32_t Thread::numCreated_(0);

/**
 * TODO:error  
 * default argument given for parameter 2 of ‘Thread::Thread(Thread::ThreadFunc, const string&)’GCC
 * 默认参数在定义和声明中只能出现一次，不能声明和定义都有默认参数
 */

Thread::Thread(ThreadFunc func, const std::string &name) :
    started_(false), // 还未开始
    joined_(false),  // 还未设置等待线程
    tid_(0),         // 初始 tid 设置为0
    func_(std::move(func)), // EventLoopThread::threadFunc()
    name_(name)     // 默认姓名是空字符串
{
    // 设置线程索引编号和姓名
    setDefaultName();
}

// 启动线程，并获取线程的tid
void Thread::start() {
    // 线程已经启动
    started_ = true;
    // 信号量，用于确保线程获取到tid
    sem_t sem;
    sem_init(&sem, false, 0);
    // 开启线程，利用智能指针自动管理内存
    thread_ = std::shared_ptr<std::thread>(new std::thread([&]() {
        // 获取线程tid
        tid_ = CurrentThread::tid();
        // v操作
        sem_post(&sem);
        // 开启一个新线程专门执行该线程函数
        func_();
    }));

    /**
     * 这里必须等待获取上面新创建的线程的tid
     * 未获取到信息则不会执行sem_post，所以会被阻塞住
     * 如果不使用信号量操作，则别的线程访问tid时候，可能上面的线程还没有获取到tid
     */
    sem_wait(&sem);
}


Thread::~Thread() {
    // 现场感启动时并且未设置等待线程时才可调用
    if (started_ && !joined_) {
        // 设置线程分离(守护线程，不需要等待线程结束，不会产生孤儿线程)
        thread_->detach();
    }
}

// 等待线程执行完毕函数
void Thread::join() {
    joined_ = true;
    // 等待线程执行完毕
    thread_->join();
}

// 设置线程默认函数
void Thread::setDefaultName() {
    int num = ++numCreated_;
    if (name_.empty()) {
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}