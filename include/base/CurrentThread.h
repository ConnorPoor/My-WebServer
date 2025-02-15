#ifndef CURRENT_THREAD_H
#define CURRENT_THREAD_H

#include "Timestamp.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#include <stdint.h>

namespace CurrentThread {
    extern __thread int t_cachedTid; // 保存tid缓冲，避免多次系统调用
    
    void cacheTid();
    void sleepUsec(int64_t usec);

    // 内联函数
    inline int tid() {
        if (__builtin_expect(t_cachedTid == 0, 0)) {
            cacheTid();
        }
        return t_cachedTid;
    }
}

#endif // CURRENT_THREAD_H