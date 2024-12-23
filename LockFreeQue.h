
/*
 * 作者:江宇豪
 * 时间:2024年12月15日22:35:35
 * 介绍:无锁的循环队列
 * */

//无锁队列（lock-free queue）是指在多个线程同时对队列进行操作时，不使用互斥锁（如mutex）来保证同步。
//无锁队列通过原子操作（atomic operations）来确保线程安全，但不需要使用传统的加锁方式，从而避免了锁带来的性能开销和潜在的死锁问题。
//无锁队列通常通过一些技巧来确保线程间不会冲突，最常见的一种方法是使用 CAS（比较并交换，Compare-And-Swap）等原子操作来保证数据的一致性。

//内存序（memory ordering）是指在多线程程序中，如何确保不同线程间的内存访问操作按正确的顺序发生。
//在多线程编程中，不同的线程可能会看到不同的内存值，尤其是当你使用缓存、寄存器和其他优化时。
//为了保证数据的一致性，必须使用正确的内存序。

//内存序通常分为几种类型：
//
//memory_order_relaxed：最宽松的顺序，不保证任何操作顺序。
//memory_order_consume：保证后续的操作依赖于当前的操作。
//memory_order_acquire：保证当前操作之后的操作不会被提前执行。 load
//memory_order_release：保证当前操作之前的操作不会被延迟执行。 store
//memory_order_acq_rel：同时包含acquire和release。
//memory_order_seq_cst：最严格的顺序，所有线程的操作必须按顺序执行。

#ifndef MIDDLEWARES_LOCKFREEQUE_H
#define MIDDLEWARES_LOCKFREEQUE_H
#include <iostream>
#include <atomic>
#include <vector>
#include <thread>
#include <cassert>
#include <chrono>


// 原子变量写入 用 store, 读取用 load
template <typename T>
class LockFreeQueue {
public:
    LockFreeQueue(size_t size)
            : size_(size),
              buffer_(size),
              head_(0),
              tail_(0) {}

    bool enqueue(const T& value) {
//        不对内存操作进行同步，也就是没有任何的跨线程顺序保证，除了原子操作本身的原子性。
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) % size_; // 留一个位子用来区分满和空
        // head_.load保证当前操作之后(读head_内容)的操作不会被提前执行
        if (next_tail == head_.load(std::memory_order_acquire)) {
            // Queue is full
            return false;
        }

        buffer_[current_tail] = value;
        // tail_ = next_tail
        // tail_ 写操作的结果会对其他线程可见，但它确保的是在写操作之前的所有操作已经完成。
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    bool dequeue(T& value) {
        size_t current_head = head_.load(std::memory_order_relaxed);

//        这里再次读取 tail_ 变量的值，使用 std::memory_order_acquire，表示这是一次获取操作，会确保 tail_ 之前的所有操作完成。
        if (current_head == tail_.load(std::memory_order_acquire)) {
            // Queue is empty
            return false;
        }

        value = buffer_[current_head];
//        这确保在更新 head_ 之前，当前线程的所有操作（包括队列消费的操作）都已完成。
        head_.store((current_head + 1) % size_, std::memory_order_release);
        return true;
    }

private:
    size_t size_;
    std::vector<T> buffer_;
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_; // 尾指针指向队尾元素的下一个位置
};

std::mutex mtx;
void producer(LockFreeQueue<int>& queue, int num_produces) {
    for (int i = 0; i < num_produces; ++i) {
        while (!queue.enqueue(i)) {
            // Retry if the queue is full
            std::this_thread::yield();
        }
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "Produced: " << i << std::endl;
    }
}

void consumer(LockFreeQueue<int>& queue, int num_consumes) {
    int value;
    for (int i = 0; i < num_consumes; ++i) {
        while (!queue.dequeue(value)) {
            // Retry if the queue is empty
            std::this_thread::yield();
        }
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "Consumed: " << value << std::endl;
    }
}

void testLockFreeQue() {
    const int queue_size = 5;
    const int num_items = 20;  // Number of items each thread will produce or consume
    LockFreeQueue<int> queue(queue_size);

    // Number of producer and consumer threads
    const int num_producers = 2;
    const int num_consumers = 2;

    std::vector<std::thread> threads;

    // Start producer threads
    for (int i = 0; i < num_producers; ++i) {
        threads.push_back(std::thread(producer, std::ref(queue), num_items));
    }

    // Start consumer threads
    for (int i = 0; i < num_consumers; ++i) {
        threads.push_back(std::thread(consumer, std::ref(queue), num_items));
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }

}


#endif //MIDDLEWARES_LOCKFREEQUE_H
