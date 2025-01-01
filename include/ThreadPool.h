//
// Created by 江宇豪 on 2025/1/1.
//

#ifndef MIDDLEWARES_THREADPOOL_H
#define MIDDLEWARES_THREADPOOL_H
#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <chrono>
#include "Semaphore.h"

// 任意数据类型的抽象基类(万能类型变量)，有点像C里的 void *，对其进行了面向对象的封装处理
// 这个类的 T 就是实际希望存储的类型
class Any
{
public:
    Any() = default;
    ~Any() = default;
    // unique_ptr禁止从左值拷贝,所以这两个构造函数应该删除
    Any(const Any&) = delete;
    Any& operator=(const Any&) = delete;
    Any(Any&&) = default;
    Any& operator=(Any&&) = default;

    // 要点2：有参构造
    template<typename T>
    Any(T data) : base_(std::make_unique<Derive<T>>(data)) {}

    // 要点3：提取出Any对象的底层实际数据
    template<typename T>
    T cast_()
    {
        // 基类转派生类指针， RTTI
        // get 获取底层的原生指针
        Derive<T> *pd = dynamic_cast<Derive<T>*>(base_.get());
        if(pd == nullptr)
        {
            throw "type is incompatible!";
        }
        return pd->data_;
    }
private:
    class Base
    {
    public:
        virtual ~Base() = default;
    };
    // 要点一
    template<typename T>
    class Derive : public Base
    {
    public:
        Derive(T data) : data_(data) {}
        T data_; // 具体的返回值类型
    };
private:
    std::unique_ptr<Base> base_;
};

class Task;
// 任务结果的抽象基类
class Result
{
public:
    Result(std::shared_ptr<Task> task, bool isValid = true);
    ~Result() = default;

    // 设置任务执行后的结果
    void setVal(Any any);

    // 用户调用该方法获得任务执行的具体结果
    Any get();

    // 获取任务执行后的返回值
private:
    Any any_; // 存储任务处理结果的返回值
    Semaphore sem_; // 线程间通信使用
    std::shared_ptr<Task> task_; // 标识结果对应的任务
    std::atomic_bool isValid_; // 检查返回值是否有效
};

// 任务的抽象基类
class Task
{
public:
    Task();
    ~Task() = default;
    // 执行任务（即用户重写的run()方法）
    void exec();
    // 设置存放结果的 Result 对象
    void setResult(Result *res);
    // 给用户提供的关键接口！！！！
    // 用户可自定义任务类型， 从Task派生而来，重写run方法，实现自定义任务处理
    virtual Any run() = 0;

private:
    // 不要用智能指针，可能与Result对象产生循环引用的问题
    Result *result_; // Resul对象生命周期 > Task对象生命周期
};

enum class PoolMode
{
    MODE_FIXED,
    MODE_CACHED,
};

class Thread
{
public:
    using ThreadFunc = std::function<void(int)>;

    Thread(ThreadFunc func);
    ~Thread();

    // 启动线程
    void start();
    int getId() const;
private:
    ThreadFunc func_;   // 线程的工作函数
    static int generateId; // 线程的Id
    int threadId_; // 保存线程对象id，不是指系统返回的线程句柄
};

class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    // 设值线程池工作模式
    void setMode(PoolMode mode);

    // 设置task任务队列上限阈值
    void setTaskQueMaxThreshhold(int threshhold);

    // 设置线程池cached模式下线程上限的阈值
    void setThreadSizeThreshhold(int threshhold);

    // 向线程池提交任务
    Result submitTask(std::shared_ptr<Task> sp);

    // 开启线程池(默认参数为CPU核数)
    void start(int initThreadSize = std::thread::hardware_concurrency());

    // 禁止拷贝构造和赋值——单例
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    // 定义线程函数
    void ThreadFunc(int threadid);

    // 检查pool的运行状态
    bool checkRunningState() const;

private:
    // 线程ID和线程指针的映射
    std::unordered_map<int, std::unique_ptr<Thread>> threads_; // 工作队列
    size_t initThreadSize_; // 初始线程数量
    int threadSizeThreshHold_; // 线程数量上限（cached模式使用）
    std::atomic_int curThreadSize_; // 当前线程池内线程总数（cached）
    std::atomic_int idleThreadSize_; // 空闲线程的数量

    // 任务队列
    std::queue<std::shared_ptr<Task>> taskQue_;
    std::atomic_int taskSize_;  // 当前任务队列中的任务数量
    int taskQueMaxThreshHold_; // 任务队列数量上限

    std::mutex taskQueMtx_; // 确保任务队列的线程安全
    std::condition_variable notFull_; // 表示任务队列不满
    std::condition_variable notEmpty_; // 表示任务队列不空
    std::condition_variable exitCond_; // 等待资源全部回收

    PoolMode poolMode_; // 线程池模式
    std::atomic_bool isPoolRunning_; // 线程池运行状态
};

// 测试函数
void TestThreadPool();
#endif //MIDDLEWARES_THREADPOOL_H
