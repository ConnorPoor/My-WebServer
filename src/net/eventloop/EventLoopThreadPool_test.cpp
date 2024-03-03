// EventLoopThreadPool_unittest.cpp
#include "EventLoop.h"
#include "EventLoopThreadPool.h"

void print(EventLoop* p = NULL) {
    printf("main(): pid=%d, tid=%d, loop=%p\n",
          getpid(), CurrentThread::tid(), p);
}

void init(EventLoop* p) {
    printf("init(): pid=%d, tid=%d, loop=%p\n",
           getpid(), CurrentThread::tid(), p);
}

int EventLoopThreadPool_test() {
    print();

    EventLoop loop;
    loop.runAfter(11, std::bind(&EventLoop::quit, &loop));

    { // 0线程的IO线程池, 默认用main线程作为baseLoop所属线程, 处理IO事件
        printf("Single thread %p:\n", &loop);
        EventLoopThreadPool model(&loop, "single"); // 0线程IO线程池
        model.setThreadNum(0);
        model.start(init); // 启动线程池并回调init
        // 从线程池连续取３次 EventLoop
        assert(model.getNextLoop() == &loop);
        assert(model.getNextLoop() == &loop);
        assert(model.getNextLoop() == &loop);
    }

    { // 单1线程的IO线程池
        printf("Another thread:\n");
        EventLoopThreadPool model(&loop, "another"); // 1个线程的IO线程池
        model.setThreadNum(1);
        model.start(init);
        EventLoop* nextLoop = model.getNextLoop();
        nextLoop->runAfter(2, std::bind(print, nextLoop));
        assert(nextLoop != &loop);
        assert(nextLoop == model.getNextLoop());
        assert(nextLoop == model.getNextLoop());
        ::sleep(3);
    }

    { // 3线程的IO线程池
        printf("Three thread:\n");
        EventLoopThreadPool model(&loop, "three");
        model.setThreadNum(3);
        model.start(init);
        EventLoop* nextLoop = model.getNextLoop();
        nextLoop->runInLoop(std::bind(print, nextLoop));
        assert(nextLoop != &loop);
        assert(nextLoop != model.getNextLoop());
        assert(nextLoop != model.getNextLoop());
        assert(nextLoop == model.getNextLoop()); // 3次以后循环回来
    }

    loop.loop();
    return 0;
}
