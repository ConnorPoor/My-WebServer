#include "EventLoopThread.h"
#include "EventLoop.h"
#include "CurrentThread.h"

void print_b(EventLoop* p = NULL) {
    printf("print: pid=%d, tid=%d, loop=%p\n",
           getpid(), CurrentThread::tid(), p);
}

void quit(EventLoop* p) {
    print_b(p);
    p->quit();
}

int EventLoopThread_test() {
    print_b();

    {
        EventLoopThread thr1; // never start
    }

    {
        // dtor calls quit()
        EventLoopThread thr2;
        EventLoop* loop = thr2.startLoop();
        loop->runInLoop(std::bind(print_b, loop));
        CurrentThread::sleepUsec(500 * 1000);
    }

    {
        // quit() before dtor
        EventLoopThread thr3;
        EventLoop* loop = thr3.startLoop();
        loop->runInLoop(std::bind(quit, loop));
        CurrentThread::sleepUsec(500 * 1000);
    }
    return 0;
}
