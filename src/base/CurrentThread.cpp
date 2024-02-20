#include "../../include/base/CurrentThread.h"

namespace CurrentThread
{
    __thread int t_cachedTid = 0;

    // 进行系统调用，获取当前进程的tid
    void cacheTid() {
        if (t_cachedTid == 0) {
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
}