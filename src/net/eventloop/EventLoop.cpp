#include "EventLoop.h"
#include "Logging.h"
#include "Poller.h"

#include <unistd.h>
#include <sys/eventfd.h>
#include <fcntl.h>

// 防止一个线程创建多个EventLoop (thread_local)
__thread EventLoop *t_loopInThisThread = nullptr;

// 定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000;

//TODO:eventfd使用
int createEventfd() {
    // 非阻塞
    int evfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evfd < 0) {
        LOG_FATAL << "eventfd error: " << errno;
    }
    return evfd;
}

EventLoop::EventLoop() : 
    looping_(false),                                // 还未开启任务循环，设置为false
    quit_(false),                                   // 还未停止事件循环
    callingPendingFunctors_(false),                 // 还未开始回调函数
    threadId_(CurrentThread::tid()),                // 获取当前线程tid
    poller_(Poller::newDefaultPoller(this)),        // 初始化poller
    timerQueue_(new TimerQueue(this)),              // 初始化定时器队列
    wakeupFd_(createEventfd()),                     // 创建eventfd作为线程间等待/通知机制
    wakeupChannel_(new Channel(this, wakeupFd_)),   // 封装wakeupFd成Channel
    currentActiveChannel_(nullptr) {                // 当前正执行的活跃Channel

    LOG_DEBUG << "EventLoop created " << this << " the index is " << threadId_;
    LOG_DEBUG << "EventLoop created wakeupFd " << wakeupChannel_->fd();
    // 已经保存了thread值，之前已经创建过了EventLoop
    // 一个线程只能对应一个EventLoop，直接退出
    if (t_loopInThisThread) {
        LOG_FATAL << "Another EventLoop" << t_loopInThisThread << " exists in this thread " << threadId_;
    } else {
        // 线程静态变量保存tid值
        t_loopInThisThread = this;
    }

    // 设置wakeupfd的事件类型以及发生事件的回调函数
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每一个EventLoop都将监听wakeupChannel的EPOLLIN事件
    // 将wakeupChannel_注册到epoll，关注读事件
    // 如果有别的线程唤醒当前EventLoop，就会向wakeupFd_写数据，触发读操作
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    // channel移除所有感兴趣事件
    wakeupChannel_->disableAll();
    // 将channel从EventLoop中删除
    wakeupChannel_->remove();
    // 关闭 wakeupFd_
    ::close(wakeupFd_);
    // 指向EventLoop指针为空
    t_loopInThisThread = nullptr;
}
  
void EventLoop::loop() {
    looping_ = true;
    quit_ = false;

    LOG_INFO << "EventLoop " << this << " start looping";

    while (!quit_) {
        // 清空activeChannels_
        activeChannels_.clear();
        // 获取发生事件的channel
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (Channel *channel : activeChannels_) {
            channel->handleEvent(pollReturnTime_);
        }
        // 执行当前EventLoop事件循环需要处理的回调操作
        /**
         * IO thread：mainLoop accept fd 打包成 chennel 分发给 subLoop
         * mainLoop实现注册一个回调，交给subLoop来执行，wakeup subLoop 之后，让其执行注册的回调操作
         * 这些回调函数在 std::vector<Functor> pendingFunctors_; 之中
         */
        doPendingFunctors();
    }
    looping_ = false;    
}

void EventLoop::quit() {
    quit_ = true;

    /**
     * TODO:生产者消费者队列派发方式和muduo的派发方式
     * 有可能是别的线程调用quit(调用线程不是生成EventLoop对象的那个线程)
     * 比如在工作线程(subLoop)中调用了IO线程(mainLoop)
     * 这种情况会唤醒主线程
     */
    if (isInLoopThread()) {
        wakeup();
    }
}

// 在当前eventLoop中执行回调函数
void EventLoop::runInLoop(Functor cb) {
    // 每个EventLoop都保存创建自己的线程tid
    // 我们可以通过CurrentThread::tid()获取当前执行线程的tid然后和EventLoop保存的进行比较
    if (isInLoopThread()) {
        cb();
    // 在非当前eventLoop线程中执行回调函数，需要唤醒evevntLoop所在线程
    } else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor cb) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb); // 使用了std::move
    }

    // 唤醒相应的，需要执行上面回调操作的loop线程
    /** 
     * TODO:
     * std::atomic_bool callingPendingFunctors_; 标志当前loop是否有需要执行的回调操作
     * 这个 || callingPendingFunctors_ 比较有必要，因为在执行回调的过程可能会加入新的回调
     * 则这个时候也需要唤醒，否则就会发生有事件到来但是仍被阻塞住的情况
     */
    if (!isInLoopThread() || callingPendingFunctors_) {
        // 唤醒loop所在的线程
        wakeup();
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::wakeup writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead() {
    // 通过写操作来唤醒epoll_wait()，形成事件驱动
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::updateChannel(Channel *channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
    return poller_->hasChannel(channel);    
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    /**
     * TODO:
     * 如果没有生成这个局部的 functors
     * 则在互斥锁加持下，我们直接遍历pendingFunctors
     * 其他线程这个时候无法访问，无法向里面注册回调函数，增加服务器时延
     */
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor &functor : functors) {
        functor();
    }

    callingPendingFunctors_ = false;
}