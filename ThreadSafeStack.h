//
// Created by 江宇豪 on 2024/10/6.
//

#ifndef MIDDLEWARES_THREADSAFESTACK_H
#define MIDDLEWARES_THREADSAFESTACK_H
#include <exception>
#include <stack>
#include <mutex>
#include <condition_variable>

struct empty_stack : std::exception
{
    const char* what() const throw()
    {
        return "Empty stack!";
    }
};



template<typename T>
class ThreadSafeStack
{
private:
    std::stack<T> data;
    mutable std::mutex m;

public:
    ThreadSafeStack(){}

    ThreadSafeStack(const ThreadSafeStack& other)
    {
        std::lock_guard<std::mutex> lock(other.m); // 保护的是 other.data
        data = other.data;
    }

    ThreadSafeStack& operator=(const ThreadSafeStack&) = delete;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }
    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack();
        std::shared_ptr<T> const res(std::make_shared<T>(std::move(data.top())));
        data.pop();
        return res;
    }

    void pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack();
        value = std::move(data.top());
        data.pop();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};



#endif //MIDDLEWARES_THREADSAFESTACK_H
