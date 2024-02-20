#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

/*
禁止拷贝操作的基类，设置为protect权限的成员函数可以让派生类继承
派生类对象可以正常的构造和析构
*/

// TODO: noncopyable 的学习
class noncopyable {
public:
    // 将拷贝构造和赋值拷贝函数定义为删除，防止派生类进行拷贝操作
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

#endif // NONCOPYABLE_H