#ifndef LOGGING_H
#define LOGGING_H

#include "Timestamp.h"
#include "LogStream.h"

#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <functional>

// SourceFile的作用是提取文件名
class SourceFile {
public:
    explicit SourceFile(const char* filename)
        : data_(filename) {
        /**
         * 找出data中出现/最后一次的位置，从而获取具体的文件名
         * 2022/10/26/test.log
         */
        const char* slash = strrchr(filename, '/');
        if (slash) {
            data_ = slash + 1;
        }
        size_ = static_cast<int>(strlen(data_));
    }

    // 文件名
    const char* data_;
    // 文件名长度
    int size_;
};

class Logger {
public:
    // 日志等级
    enum LogLevel {
        TRACE,          // 指出比DEBUG粒度更细的一些信息事件（开发过程中使用）
        DEBUG,          // 指出细粒度信息事件对调试应用程序是非常有帮助的（开发过程中使用）
        INFO,           // 表明消息在粗粒度级别上突出强调应用程序的运行过程
        WARN,           // 系统能正常运行，但可能会出现潜在错误的情形
        ERROR,          // 指出虽然发生错误事件，但仍然不影响系统的继续运行
        FATAL,          // 指出每个严重的错误事件将会导致应用程序的退出
        LEVEL_COUNT,
    };

    // member function
    Logger(const char* file, int line);
    Logger(const char* file, int line, LogLevel level);
    Logger(const char* file, int line, LogLevel level, const char* func);
    ~Logger();

    // 流是会改变的
    LogStream& stream() { return impl_.stream_; }

    // TODO:static关键字作用的函数必须在源文件实现?
    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    // 输出函数和刷新缓冲区函数
    using OutputFunc = std::function<void(const char* msg, int len)>;
    using FlushFunc = std::function<void()>;
    // 设置输出缓冲区函数
    static void setOutput(OutputFunc);
    // 设置刷新缓冲区函数
    static void setFlush(FlushFunc);

private:
    // 内部类
    class Impl {
    public:
        using LogLevel = Logger::LogLevel;
        Impl(LogLevel level, int savedErrno, const char* file, int line);
        // 根据时区格式化当前时间字符串, 也是一条log消息的开头
        void formatTime();
        // 添加一条log消息的后缀
        void finish();

        // 时间戳
        Timestamp time_;
        // 用于格式化用户log数据, 提供operator<<接口, 保存log消息
        LogStream stream_;
        // 日志等级
        LogLevel level_;
        // 源代码所在行
        int line_;
        // 写入的文件
        SourceFile basename_;
    };

    // 内部类成员变量
    Impl impl_;
};

// 日志级别
extern Logger::LogLevel g_logLevel;

// 返回日志级别
inline Logger::LogLevel logLevel() {
    return g_logLevel;
}

// 获取errno信息
const char* getErrnoMsg(int savedErrno);

/**
 * 当日志等级小于对应等级才会输出
 * 比如设置等级为FATAL，则logLevel等级大于DEBUG和INFO，DEBUG和INFO等级的日志就不会输出
 */
#define LOG_DEBUG if (logLevel() <= Logger::DEBUG) \
  Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (logLevel() <= Logger::INFO) \
  Logger(__FILE__, __LINE__).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()


#endif // LOGGING_H