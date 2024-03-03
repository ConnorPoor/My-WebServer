#include "ThreadPool.h"
#include "Timestamp.h"

int ThreadPool_test();
int Timestamp_test();
int Logger_test(int argc, char* argv[]);
int InetAddress_test();
int EventLoopThreadPool_test();
int EventLoopThread_test();

int main(int argc, char* argv[]) {
    // ThreadPool_test();
    // Timestamp_test();
    // Logger_test(argc, argv);
    // InetAddress_test();
    EventLoopThreadPool_test();
    // EventLoopThreadPool_test();

    return 0;
}