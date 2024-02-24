#ifndef LOG_STREAM_H
#define LOG_STREAM_H

#include "FixedBuffer.h"
#include "noncopyable.h"

#include <string>

/**
 *  比如SourceFile类和时间类就会用到
 *  const char* data_;
 *  int size_;
 */
class GeneralTemplate : noncopyable {
public:
    GeneralTemplate()
        : data_(nullptr),
          len_(0)
    {}

    // 禁止隐式转换
    explicit GeneralTemplate(const char* data, int len)
        : data_(data),
          len_(len)
    {}

    const char* data_;
    int len_;
};

// 日志流类，主要提供<<操作
class LogStream : noncopyable {
public:
    using Buffer = FixedBuffer<kSmallBuffer>;
    
    // 添加数据
    void append(const char* data, int len) { buffer_.append(data, len); }
    // 返回缓冲区类
    const Buffer& buffer() const { return buffer_; }
    // 重置缓冲区
    void resetBuffer() { buffer_.reset(); }

    /**
     * 我们的LogStream需要重载运算符
     */
    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);

    LogStream& operator<<(float v);
    LogStream& operator<<(double v);

    LogStream& operator<<(char c);
    LogStream& operator<<(const void* data);
    LogStream& operator<<(const char* str);
    LogStream& operator<<(const unsigned char* str);
    LogStream& operator<<(const std::string& str);
    LogStream& operator<<(const Buffer& buf);

    // (const char*, int)的重载
    LogStream& operator<<(const GeneralTemplate& g);

private:
    // 最大数字长度
    static const int kMaxNumericSize = 48;

    // 对于整型需要特殊处理
    template <typename T>
    void formatInteger(T);

    // 缓冲区
    Buffer buffer_;
};

#endif // LOG_STREAM_H