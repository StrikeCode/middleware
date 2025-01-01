/*
 * 作者:江宇豪
 * 时间:2024年08月18日 17:51:46
 * 介绍:线程安全的饿汉单例模式模板
 * */
#ifndef MIDDLEWARES_SINGLETION_H
#define MIDDLEWARES_SINGLETION_H
#include <mutex>
#include <memory>

template<typename T>
class Singleton
{
public:
    static T& Instance()
    {
        std::call_once(init_instance_flag_, &Singleton::InitSingleton); // 线程安全的单例
        return *instance_;
    }
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

protected:
    struct ObjectCreator
    {
        ObjectCreator() { Singleton<T>::Instance(); }
        inline void DoNothing() const {}
    };
    // 确保单例对象第一次使用前已经初始化，饿汉实现
    static ObjectCreator create_object_;
    Singleton() {} // 普通构造写在这里是为了提高灵活性，可能在工厂模式中有用。也能写在 private下
    virtual ~Singleton() {}
private:
    static void InitSingleton()
    {
        instance_.reset(new T()); // 创建一个新的 T 类型对象，并将其指针交给 instance_
    }

    static std::unique_ptr<T> instance_; // T类型对象只能有唯一的所有者，符合单例设计意图，保证单例唯一性
    static std::once_flag init_instance_flag_; // 记录 InitSingleton 操作是否被执行过
};

template <typename T>
std::unique_ptr<T> Singleton<T>::instance_ = nullptr;

template <typename T>
std::once_flag Singleton<T>::init_instance_flag_;

template <typename T>
typename Singleton<T>::ObjectCreator Singleton<T>::create_object_; // 静态成员，类内声明，类外初始化

// How To Use:
/*
class Logger : public Singleton<Logger>
{
public:
    void Log(const std::string& message)
    {
        std::cout << "Log: " << message << std::endl;
    }
private:
    // 防止外部创建实例，在使用进行控制。这个策略很好，没有在单例代码里做
    Logger() {}
    friend class Singleton<Logger>; // 允许Singleton访问 Logger 的私有构造函数 InitSingleton
};


int main()
{
    Logger::Instance().Log("Singleton pattern example");
    Logger::Instance().Log("Another");

    return 0;
}*/
#endif //MIDDLEWARES_SINGLETION_H

