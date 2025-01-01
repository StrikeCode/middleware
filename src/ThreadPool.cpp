//
// Created by 江宇豪 on 2025/1/1.
//
#include "../include/ThreadPool.h"
#include <iostream>
#include <functional>
#include <thread>

const int TASK_MAX_THRESHHOLD = INT32_MAX;
const int THREAD_MAX_THRESHHOLD = 1024;
const int THREAD_MAX_IDLE_TIME = 60; // 线程最大空闲时间，超过则回收
/////////////////////// ThreadPool
ThreadPool::ThreadPool()
        : initThreadSize_(4),
          taskSize_(0),
          idleThreadSize_(0),
          curThreadSize_(0),
          taskQueMaxThreshHold_(TASK_MAX_THRESHHOLD),
          threadSizeThreshHold_(THREAD_MAX_THRESHHOLD),
          poolMode_(PoolMode::MODE_FIXED),
          isPoolRunning_(false)
{
}
ThreadPool::~ThreadPool()
{
    isPoolRunning_ = false;
    // 等待线程池里所有线程返回，两种状态：阻塞 或 正在执行任务中
    std::unique_lock<std::mutex> lock(taskQueMtx_);
    // 通知所有线程，它们检测到线程池退出状态，也退出工作
    notEmpty_.notify_all();
    // 等待线程退出，每个线程退出都会notify一次，但是必须要谓词为true，才可以重新抢锁，解除阻塞
    exitCond_.wait(lock, [&]() -> bool
    { return threads_.size() == 0; });
}

// 设值线程池工作模式
void ThreadPool::setMode(PoolMode mode)
{
    // 正在运行中，不允许修改模式
    if (checkRunningState())
        return;
    poolMode_ = mode;
}

// 设置task任务队列上限阈值
void ThreadPool::setTaskQueMaxThreshhold(int threshhold)
{
    if (checkRunningState())
        return;
    taskQueMaxThreshHold_ = threshhold;
}

// 设置线程池cached模式下线程上限的阈值
void ThreadPool::setThreadSizeThreshhold(int threshhold)
{
    if (checkRunningState())
        return;
    if(poolMode_ == PoolMode::MODE_CACHED)
        threadSizeThreshHold_ = threshhold;
}

// 向线程池提交任务
// 这是一个队列的操作，需要对其进行加锁操作
Result ThreadPool::submitTask(std::shared_ptr<Task> sp)
{
    std::unique_lock<std::mutex> lck(taskQueMtx_);

    // 等价于上面的while
    // 满足第二项的(任务队列不超过上限) pred才能退出内部的while
    if(!notFull_.wait_for(lck, std::chrono::seconds(1), [&]()->bool {return taskQue_.size() < (size_t)taskQueMaxThreshHold_ ;}))
    {
        // 超时了就打印错误信息
        std::cerr << "task queue is full, submit task fail." << std::endl;
        return Result(sp, false);
    }

    // 队列有空余，把任务放到队列中
    taskQue_.emplace(sp);
    taskSize_++;

    // 通知消费者消费
    notEmpty_.notify_all();

    // 注意：是在提交任务时进行线程扩充,每次提交就最多扩充一个线程
    // cached模式适用于，任务处理比较紧急，且任务小而快
    // 需要同时根据 任务数量 和 空闲线程数量，判断是否需要创建新的线程
    if(poolMode_ == PoolMode::MODE_CACHED
       && taskSize_ > idleThreadSize_
       && curThreadSize_ < threadSizeThreshHold_)
    {
        std::cout << ">>> create new thread..." << std::endl;

        // 创建新的线程对象
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1));
        int threadId = ptr->getId(); // 这是我们自定义的线程编号，不是调用线程创建函数系统返回给我们的线程号
        // 记录线程编号和线程的映射
        threads_.emplace(threadId, std::move(ptr));

        // 启动线程
        threads_[threadId]->start();

        curThreadSize_++;
        idleThreadSize_++;
    }

    // 返回任务的执行结果对象
    return Result(sp);
}

// 开启线程池(默认参数为CPU核数)
void ThreadPool::start(int initThreadSize)
{
    // 设置线程池运行状态
    isPoolRunning_ = true;

    // 记录初始线程个数
    initThreadSize_ = initThreadSize;
    curThreadSize_ = initThreadSize;

    // 创建线程对象
    // 把线程函数给到 thread 线程对象
    for(size_t i = 0; i < initThreadSize_; i++)
    {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1));
        int threadId = ptr->getId(); // 这是我们自定义的线程编号，不是调用线程创建函数系统返回给我们的线程号
        // 记录线程编号和线程的映射
        threads_.emplace(threadId, std::move(ptr));
    }

    // 启动线程
    for(size_t i = 0; i < initThreadSize_; i++)
    {
        threads_[i]->start(); // 执行一个线程函数
        idleThreadSize_++;
    }
}

// 定义线程工作函数
// 参数是线程池中的某一个线程的编号
void ThreadPool::ThreadFunc(int threadid)
{
    auto lastTime = std::chrono::high_resolution_clock().now();

    // 所有任务必须执行完成，线程池才可以回收所有线程资源
    for(;;)
    {
        std::shared_ptr<Task> task;
        {
            // 获取锁
            std::unique_lock<std::mutex> lock(taskQueMtx_);
            std::cout << "tid:" << std::this_thread::get_id() << "尝试获取任务..." << std::endl;

            // 没有任务的处理
            while(taskQue_.size() == 0)
            {
                // 线程池停止运行，回收线程资源
                if(!isPoolRunning_)
                {
                    threads_.erase(threadid);
                    std::cout << "threadid:" << std::this_thread::get_id() << " exit!" << std::endl;
                    // 每个线程退出时都会 唤醒一次 exitCond_.wait
                    exitCond_.notify_all();
                    return ;
                }
                // 若为可伸缩模式，则检查是否空转超时且当前开的线程超过了初始线程
                // 若超过了 ,则回收线程
                if(poolMode_ == PoolMode::MODE_CACHED)
                {
                    // 每秒检测一次队列是否有任务
                    if(std::cv_status::timeout ==
                       notEmpty_.wait_for(lock, std::chrono::seconds(1)))
                    {
                        // 队列没任务
                        auto now = std::chrono::high_resolution_clock().now();
                        auto dur = std::chrono::duration_cast<std::chrono::seconds> (now - lastTime); // 计算没任务的的持续时间

                        // 超过空闲限制，并且有生成超过初始线程数量的线程数,则退删除当前线程（只是删除映射表的记录）
                        if(dur.count() >= THREAD_MAX_IDLE_TIME
                           && curThreadSize_ > initThreadSize_)
                        {
                            // 回收当前线程
                            // 记录线程数量相关变量的修改
                            // 将线程对象从线程列表容器中删除
                            // threadid -> Thread 对象 -> erase
                            threads_.erase(threadid);
                            curThreadSize_--;
                            idleThreadSize_--;

                            std::cout << "threadid:" << std::this_thread::get_id();
                            return ;
                        }
                    }
                }
                else // fixed 模式
                {
                    notEmpty_.wait(lock); // 1. unlock 2. wait for signal 3. lock
                }
            }

            idleThreadSize_--; // 执行任务，空闲线程数-1

            std::cout << "tid:" << std::this_thread::get_id() << "获取任务成功..." << std::endl;
            // 从任务队列中获得一个任务
            task = taskQue_.front();
            taskQue_.pop();
            taskSize_--;

            // 有剩余任务，继续通知其他线程执行任务
            if(taskQue_.size() > 0)
            {
                notEmpty_.notify_all();
            }

            // 取出一个任务后，位置肯定是有空缺的通知消费者可以生产
            notFull_.notify_all();
        }

        // 当前线程负责执行该任务
        if(task != nullptr)
        {
            // 执行任务: 内部调用run方法，并且设置工作结果对象
            task->exec();
        }
    }
}

// 检查pool的运行状态
bool ThreadPool::checkRunningState() const
{
    return isPoolRunning_;
}

/////////////////////// Thread
int Thread::generateId = 0; // 静态成员，类内声明类外初始化
Thread::Thread(ThreadFunc func)
: func_(func) // 传的就是 std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1) 生成的 function object
, threadId_(generateId++)
{

}
Thread::~Thread()
{

}

// 启动线程
void Thread::start()
{
    std::thread t(func_, threadId_);
    t.detach();
}
int Thread::getId() const
{
    return threadId_;
}

/////////////////////// Task
Task::Task()
        : result_(nullptr)
{

}
// 执行任务（即用户重写的run()方法）
void Task::exec()
{
    if(nullptr != result_)
    {
        // run是留给用户进行重写实现自己想要的功能
        result_->setVal(run());
    }
}
// 设置存放结果的 Result 对象
void Task::setResult(Result *res)
{
    result_ = res;
}

/////////////////////// Result
Result::Result(std::shared_ptr<Task> task, bool isValid)
        : task_(task)
        , isValid_(isValid)
{
    task_->setResult(this);
}


// 设置任务执行后的结果
void Result::setVal(Any any)
{
    // Any类型只有移动赋值和移动构造，所以必须用 std::move
    this->any_ = std::move(any);
    sem_.post();
}

// 用户调用该方法获得任务执行的具体结果
Any Result::get()
{
    if(!isValid_) return "";
    sem_.wait();
    return std::move(any_);
}

// test

using uLong = unsigned long long;
// 使用 3条线程来计算1 + 2 + 3 + ... + 3 0000 0000 (一加到三亿)
// 使用示例：实现Task的子类并且对其进行重写其中的run方法，就是其对应的工作函数
class MyTask : public Task
{
public:
    MyTask(uLong begin, uLong end)
            : begin_(begin)
            , end_(end)
    {}

    Any run()
    {
        std::cout << "tid:" << std::this_thread::get_id() << "begin!" << std::endl;
        uLong sum = 0;
        for(uLong i = begin_; i <= end_; i++)
        {
            sum += i;
        }

        std::cout << "tid:" << std::this_thread::get_id() << "end!" << std::endl;
        return sum;
    }

private:
    uLong begin_;
    uLong end_;
};

void TestThreadPool()
{
    ThreadPool pool;
    // 设置为线程数可伸缩模式
    pool.setMode(PoolMode::MODE_CACHED);
    pool.start(4);

    // exec -> setval(run())
    Result res1 = pool.submitTask(std::make_shared<MyTask>(1, 100000000));
    Result res2 = pool.submitTask(std::make_shared<MyTask>(100000001, 200000000));
    Result res3 = pool.submitTask(std::make_shared<MyTask>(200000001, 300000000));

    // 获取计算结果
    uLong sum1 = res1.get().cast_<uLong>();
    uLong sum2 = res2.get().cast_<uLong>();
    uLong sum3 = res3.get().cast_<uLong>();

    // Master - Slave线程模型
    std::cout << (sum1 + sum2 + sum3) << std::endl;
}