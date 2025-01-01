// 条件变量配合互斥锁实现信号量
// Created by 江宇豪 on 2024/9/1.
//

#ifndef MIDDLEWARES_SEMAPHORE_H
#define MIDDLEWARES_SEMAPHORE_H
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

#if 0 // #1 implementation
class Semaphore {
public:
    // 单参数构造函数加 explicit 防止无意的隐式转换
    explicit Semaphore(int count = 0) : count_(count) {};
    void Signal()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        count_++;
        cv_.notify_one();
    }
    void Wait()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        // 先解锁，然后loop，直到 pred为true则退出loop，重新lock
        cv_.wait(lock, [this](){ return count_ > 0;});
        count_--;
    }
private:
    std::mutex mtx_;
    std::condition_variable cv_;
    int count_;
};
#endif

// 互斥锁 + 条件变量 实现 信号量 用于ThreadPool.h
class Semaphore
{
public:
    Semaphore(int limit = 0) : resLimit_(limit)
    {}
    ~Semaphore() = default;

    // 获取一个信号量资源，名字也可改为 signal
    void wait()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        // lambda返回若为false就循环wait
        cond_.wait(lock, [&]()->bool { return resLimit_ > 0;});
        resLimit_--;
    }
    // 增加一个信号量资源
    void post()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        resLimit_++;
        // 会唤醒所有等待队列中阻塞的线程，但是存在锁争用,只有一个线程能获得锁
        cond_.notify_all();
    }
private:
    int resLimit_;
    std::mutex mtx_;
    std::condition_variable cond_;
};
// How To Use:
/*Semaphore Sem(1); // Initial count is 1 equal to mutex

// 每个线程持有信号量1s
void worker(int id)
{
    Sem.Wait();
    std::cout << "Worker " << id << " is in critical section." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Worker " << id << " is leaving critical section." << std::endl;
    Sem.Signal();
}
int main()
{
    std::vector<std::thread> threads;

    for(int i = 0; i < 5; ++i)
    {
        threads.push_back(std::thread(worker, i));
    }

    for(auto& thread : threads)
    {
        thread.join();
    }

    return 0;
}*/

#endif //MIDDLEWARES_SEMAPHORE_H
