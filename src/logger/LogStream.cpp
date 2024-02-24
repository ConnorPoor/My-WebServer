#include "LogStream.h"

#include <algorithm>

// 定义数字字符数组，左右两边都有1-9保证正负数都可使用
static const char digits[] = {'9', '8', '7', '6', '5', '4', '3', '2', '1', '0',
                               '1', '2', '3', '4', '5', '6', '7', '8', '9'};

// 对于十进制整型，如int/long等，通过模板函数formatInteger()，将转换为字符串并直接填入Small Buffer尾部
template <typename T>
void LogStream::formatInteger(T num) {
    if (buffer_.avail() >= kMaxNumericSize) {
        char* start = buffer_.current();
        char* cur = start;
        const char* zero = digits + 9;
        bool negative = (num < 0); // 是否为负数

        // 末尾取值加入，最后反转
        do {
            int remainder = static_cast<int>(num % 10);
            *(cur++) = zero[remainder];
            num = num / 10;
        } while (num != 0);

        if (negative) {
            *(cur++) = '-';
        }
        *cur = '\0';

        std::reverse(start, cur);
        buffer_.add(static_cast<int>(cur - start)); // cur_向后移动
    }
}

// short类型转换为int型写入
LogStream& LogStream::operator<<(short v) {
    *this << static_cast<int>(v);
    return *this;
}

// unsigned int类型转换为unsigned int型写入
LogStream& LogStream::operator<<(unsigned short v) {
    *this << static_cast<unsigned int>(v);
    return *this;
}

// 通过模板函数写入
LogStream& LogStream::operator<<(int v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
    formatInteger(v);
    return *this;
}

// float类型转换为double类型写入
LogStream& LogStream::operator<<(float v) 
{
    *this << static_cast<double>(v);
    return *this;
}

// 将v转换为字符串, 并填入buffer_当前尾部. %g 自动选择%f, %e格式, 并且不输出无意义0. %.12g 最多保留12位小数
LogStream& LogStream::operator<<(double v) {
    if (buffer_.avail() >= kMaxNumericSize) {
        char buf[32];
        int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v); 
        buffer_.add(len);
    }
    return *this;
}

// char字符直接写入
LogStream& LogStream::operator<<(char c) {
    buffer_.append(&c, 1);
    return *this;
}

// const void*类型转换为cosnt char*类型写入
LogStream& LogStream::operator<<(const void* data)  {
    *this << static_cast<const char*>(data); 
    return *this;
}

// 字符串类型判断是否为空，若为空则写入"(null)"
LogStream& LogStream::operator<<(const char* str) {
    if (str) {
        buffer_.append(str, strlen(str));
    } else {
        buffer_.append("(null)", 6);
    }
    return *this;
}

// unsigned char*转换为const char*写入
LogStream& LogStream::operator<<(const unsigned char* str) {
    return operator<<(reinterpret_cast<const char*>(str));
}

// string类型通过c_str()转换成字符指针直接写入
LogStream& LogStream::operator<<(const std::string& str) {
    buffer_.append(str.c_str(), str.size());
    return *this;
}

// 另一个缓冲区通过toString()函数转换成string类型写入
LogStream& LogStream::operator<<(const Buffer& buf) {
    *this << buf.toString();
    return *this;
}

// 通用接口（包含data_，以及len_类型）写入
LogStream& LogStream::operator<<(const GeneralTemplate& g) {
    buffer_.append(g.data_, g.len_);
    return *this;
}