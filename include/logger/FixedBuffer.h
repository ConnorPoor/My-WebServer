#ifndef FIXED_BUFFER_H
#define FIXED_BUFFER_H

#include "noncopyable.h"

#include <assert.h>
#include <string.h> // memcpy
#include <strings.h>
#include <string>

// 小块缓冲区大小，4KB
const int kSmallBuffer = 4000;
// 大块缓冲区大小，4MB
const int kLargeBuffer = 4000*1000; 

template <int SIZE>
class FixedBuffer : noncopyable {
public:
    FixedBuffer()
        : cur_(data_)
    {}

    // 向缓冲区里面加入数据
    void append(const char* buf, size_t len) {
        if (static_cast<size_t>(avail()) > len) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    // 缓冲区开头指针
    const char* data() const { return data_; }
    // 缓冲区大小
    int length() const { return static_cast<int>(end() - data_); }

    // 当前写入位置指针
    char* current() { return cur_; }
    // 剩余大小
    int avail() const { return static_cast<int>(end() - cur_); }
    // 加入len长度的数据
    void add(size_t len) { cur_ += len; }

    // 重置缓冲区
    void reset() { cur_ = data_; }
    // 将缓冲区数据都置0
    void bzero() { ::bzero(data_, sizeof(data_)); }

    // 转换成string类型数据
    std::string toString() const { return std::string(data_, length()); }

private:
    // 缓冲区最后一位的后面一位，相当于STL迭代器中end的位置
    const char* end() const { return data_ + sizeof(data_); }

    // 缓冲区字符数组
    char data_[SIZE];
    // 当前写到的位置
    char* cur_; 
};


#endif // FIXED_BUFFER_H