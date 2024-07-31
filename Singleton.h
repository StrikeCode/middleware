// C++单例模板类
// 这一版自己抄一下看看可不可以用
// 还没有测试
#include <mutex>
#include <memory>

template <typename T>
class Singleton {
public:
    static T& Instance() {
        std::call_once(initInstanceFlag, &Singleton::InitSingleton);
        return *instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

protected:
    struct object_creator {
        object_creator() { Singleton<T>::Instance(); }
        inline void do_nothing() const {}
    };
    static object_creator create_object;

    Singleton() {}
    virtual ~Singleton() {}

private:
    static void InitSingleton() {
        instance.reset(new T());
    }

    static std::unique_ptr<T> instance;
    static std::once_flag initInstanceFlag;
};

template <typename T>
std::unique_ptr<T> Singleton<T>::instance = nullptr;

template <typename T>
std::once_flag Singleton<T>::initInstanceFlag;

template <typename T>
typename Singleton<T>::object_creator Singleton<T>::create_object;