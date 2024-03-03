#include "CurrentThread.h"

namespace CurrentThread
{
    __thread int t_cachedTid = 0;

    // 进行系统调用，获取当前进程的tid
    void cacheTid() {
        if (t_cachedTid == 0) {
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }

    void sleepUsec(int64_t usec)
    {
        struct timespec ts = {0, 0};
        ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);

    //    std::this_thread::sleep_for(std::chrono::microseconds());
        ::nanosleep(&ts, NULL);
    }
}