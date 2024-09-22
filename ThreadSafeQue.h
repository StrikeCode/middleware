//
// Created by 江宇豪 on 2024/9/17.
//

#ifndef MIDDLEWARES_THREADSAFEQUE_H
#define MIDDLEWARES_THREADSAFEQUE_H
#include <mutex>
#include <queue>

template<typename T>
class ThreadSafeQueue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;

public:
    ThreadSafeQueue(){}

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(std::move(new_value));
        data_cond.notify_one();
    }

    void wait_and_pop(T &value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_queue.empty();});
        value = std::move(data_queue.front());
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut); // 支持锁的解锁和重新锁定，
        data_cond.wait(lk, [this]{return !data_queue.empty();});
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool try_pop(T &value)
    {
        std::unique_lock<std::mutex> lk(mut);
        if(data_queue.empty())
            return false;
        value = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        if(data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }

};

template<typename T>
class ThreadSafeQueuePtr
{
private:
    mutable std::mutex mtx;
    std::queue<std::shared_ptr<T>> data_queue;
    std::condition_variable data_cond;

public:
    ThreadSafeQueuePtr()
    {}
    void push(T new_value)
    {

    }

    void wait_and_pop(T &value)
    {

    }

    std::shared_ptr<T> wait_and_pop()
    {

    }

    bool try_pop(T &value)
    {

    }

    bool empty() const
    {

    }
};

template<typename T>
class ThreadSafeQueueHt
{
private:
    struct node
    {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node *tail;
    std::condition_variable data_cond;

    node * get_tail()
    {

    }
    std::unique_ptr<node> pop_head()
    {

    }

    std::unique_lock<std::mutex> wait_for_data()
    {

    }

    std::unique_ptr<node> wait_pop_head()
    {

    }
    std::unique_ptr<node> wait_pop_head(T &value)
    {

    }
    std::unique_ptr<node> try_pop_head()
    {

    }

    std::unique_ptr<node> try_pop_head(T &value)
    {

    }

public:
    ThreadSafeQueueHt()
    {

    }

    ThreadSafeQueueHt(const ThreadSafeQueueHt &other) = delete;

    ThreadSafeQueueHt& operator=(const ThreadSafeQueueHt &other) = delete;

    void push(T new_value)
    {

    }

    void wait_and_pop(T &value)
    {

    }

    std::shared_ptr<T> wait_and_pop()
    {

    }

    bool try_pop(T &value)
    {

    }

    bool empty() const
    {

    }
};


#endif //MIDDLEWARES_THREADSAFEQUE_H

// How to Use
//void TestThreadSafeQueue()
//{
//    std::srand(static_cast<unsigned>(std::time(nullptr)));
//    ThreadSafeQueue<int> que;
//    for(int i = 0; i < 10; ++i)
//    {
//        que.push(std::rand() % 100);
//    }
//    int val;
//    while(!que.empty())
//    {
//        std::shared_ptr<int> pVal = que.wait_and_pop();
//        std::cout << *pVal << " " << std::endl;
//    }
//}