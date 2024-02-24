#include "ThreadPool.h"
#include "CurrentThread.h"

#include <iostream>
#include <stdio.h>

using namespace std;

void print() {
    printf("tid=%d\n", CurrentThread::tid());
}

void printString(const std::string& str) {
    cout << str;
}

void test(int maxSize) {
    cout << "Test ThreadPool with max queue size = " << maxSize << '\n';
    ThreadPool pool("MainThreadPool");
    pool.setThreadSize(maxSize);
    pool.start();

    cout << "Adding\n";
    pool.add(print);
    pool.add(print);
    for (int i = 0; i < 100; ++i) {
        char buf[32];
        snprintf(buf, sizeof(buf), "task %d", i);
        pool.add(std::bind(printString, std::string(buf))); // 演示了如何向线程池加入含参的可调用对象
    }
    cout << "Done\n" << endl;
    
    // 演示了如何等待线程池运行完用户任务
    pool.stop();
}

int ThreadPool_test() {
    test(0);
    test(1);
    test(5);
    test(10);
    test(50);
    return 0;
}