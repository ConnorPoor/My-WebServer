#include "Logging.h"
#include "CurrentThread.h"

// ThreadInfo命名空间，包含输出所需的一些线程信息
namespace ThreadInfo {
    __thread char t_errnobuf[512];
    __thread char t_time[64];
    __thread time_t t_lastSecond;
};

// 获取errno信息
const char* getErrnoMsg(int savedErrno) {
    return strerror_r(savedErrno, ThreadInfo::t_errnobuf, sizeof(ThreadInfo::t_errnobuf));
}

// 根据Level返回Level名字，名字长度都为6
const char* getLevelName[Logger::LogLevel::LEVEL_COUNT] {
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
};

// 初始化日志等级
Logger::LogLevel initLogLevel() {
    return Logger::INFO;
}

// 日志等级初始化为INFO
Logger::LogLevel g_logLevel = initLogLevel();

// 默认输出（stdout）
static void defaultOutput(const char* data, int len) {
    fwrite(data, len, sizeof(char), stdout);
}

// 刷新默认（stdout）缓冲区
static void defaultFlush() {
    fflush(stdout);
}

// 默认输出函数
Logger::OutputFunc g_output = defaultOutput;
// 默认刷新缓冲区函数
Logger::FlushFunc g_flush = defaultFlush;

// Impl类构造函数，
Logger::Impl::Impl(Logger::LogLevel level, int savedErrno, const char* file, int line)
    : time_(Timestamp::now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(file) {
    // 输出流 -> time
    formatTime();
    // 写入日志等级
    stream_ << GeneralTemplate(getLevelName[level], 6);
    // TODO:error
    if (savedErrno != 0) {
        stream_ << getErrnoMsg(savedErrno) << " (errno=" << savedErrno << ") ";
    }
}

// Timestamp::toString方法的思路，只不过这里需要输出到流
void Logger::Impl::formatTime() {
    Timestamp now = Timestamp::now();
    time_t seconds = static_cast<time_t>(now.microSecondsSinceEpoch() / Timestamp::kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(now.microSecondsSinceEpoch() % Timestamp::kMicroSecondsPerSecond);

    struct tm *tm_time = localtime(&seconds);
    // 写入此线程存储的时间buf中
    snprintf(ThreadInfo::t_time, sizeof(ThreadInfo::t_time), "%4d/%02d/%02d %02d:%02d:%02d",
        tm_time->tm_year + 1900,
        tm_time->tm_mon + 1,
        tm_time->tm_mday,
        tm_time->tm_hour,
        tm_time->tm_min,
        tm_time->tm_sec);
    // 更新最后一次时间调用
    ThreadInfo::t_lastSecond = seconds;

    // muduo使用Fmt格式化整数，这里我们直接写入buf
    char buf[32] = {0};
    snprintf(buf, sizeof(buf), "%06d ", microseconds);

    // 输出时间，附有微妙(之前是(buf, 6),少了一个空格)
    stream_ << GeneralTemplate(ThreadInfo::t_time, 17) << GeneralTemplate(buf, 7);
}

void Logger::Impl::finish() {
    stream_ << " - " << GeneralTemplate(basename_.data_, basename_.size_) 
            << ':' << line_ << '\n';
}

// level默认为INFO等级
Logger::Logger(const char* file, int line)  
    : impl_(INFO, 0, file, line)
{ }

Logger::Logger(const char* file, int line, Logger::LogLevel level)
    : impl_(level, 0, file, line)
{ }

// 可以打印调用函数
Logger::Logger(const char* file, int line, Logger::LogLevel level, const char* func)
  : impl_(level, 0, file, line) {
    impl_.stream_ << func << ' ';
}


Logger::~Logger() {
    impl_.finish();
    // 获取buffer(stream_.buffer_)
    const LogStream::Buffer& buf(stream().buffer());
    // 输出(默认向终端输出)
    g_output(buf.data(), buf.length());
    // FATAL情况终止程序
    if (impl_.level_ == FATAL) {
        g_flush();
        abort();
    }
}

// 设置日志等级
void Logger::setLogLevel(Logger::LogLevel level) {
    g_logLevel = level;
}

// 设置输出函数
void Logger::setOutput(OutputFunc out) {
    g_output = out;
}

// 设置刷新缓冲区函数
void Logger::setFlush(FlushFunc flush) {
    g_flush = flush;
}