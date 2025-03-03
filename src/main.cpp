#include <iostream>
#include <cstdlib>
#include <ctime>
#include "../include/Singleton.h"
#include "../include/Semaphore.h"
#include "../include/ThreadSafeQue.h"
#include "../include/ThreadSafeStack.h"
#include "../include/LockFreeQue.h"
#include "../include/ThreadPool.h"
#include "../include/RingLog.h"
#include "../include/json11.h"

using namespace std;

class MyClass
{
public:
    MyClass(int data):_data(data){}
    MyClass(const MyClass &mc):_data(mc._data){}
    MyClass(MyClass &&mc):_data(mc._data)
    {}

    friend std::ostream& operator <<(std::ostream &os, const MyClass &mc)
    {
        os << mc._data;
        return os;
    }
private:
    int _data;
};
std::mutex mtx_cout; // 保证输出清晰的互斥量
void PrintMyClass(std::string consumer, std::shared_ptr<MyClass> data)
{
    std::lock_guard<std::mutex> lock(mtx_cout);
    std::cout << consumer << " pop data success , data is " << (*data) << std::endl;
}
#if 0 // 测试 线程安全的栈的并发操作，两个线程取，一个线程生产
void TestThreadSafeStack()
{
    threadsafe_stack_waitable<MyClass> stack;

    std::thread consumer1(
            [&]()
            {
                for(;;)
                {
                    std::shared_ptr<MyClass> data = stack.wait_and_pop();
                    PrintMyClass("consumer1", data);
                }
            }
            );

    std::thread consumer2(
            [&]()
            {
                for(;;)
                {
                    std::shared_ptr<MyClass> data = stack.wait_and_pop();
                    PrintMyClass("consumer2", data);
                }
            }
    );

    std::thread producer(
            [&]()
            {
                for(int i = 0; i < 100; i++)
                {
                    MyClass mc(i);
                    stack.push(std::move(mc));
                }
            }
            );

    consumer1.join();
    consumer2.join();
    producer.join();
}

void TestThreadSafeQue()
{
    ThreadSafeQueuePtr<MyClass> safe_que;

    std::thread consumer1(
            [&]()
            {
                for(;;)
                {
                    std::shared_ptr<MyClass> data = safe_que.wait_and_pop();
                    PrintMyClass("consumer1", data);
                }
            }
    );

    std::thread consumer2(
            [&]()
            {
                for(;;)
                {
                    std::shared_ptr<MyClass> data = safe_que.wait_and_pop();
                    PrintMyClass("consumer2", data);
                }
            }
    );

    std::thread producer(
            [&]()
            {
                for(int i = 0; i < 100; i++)
                {
                    MyClass mc(i);
                    safe_que.push(std::move(mc));
                }
            }
    );

    consumer1.join();
    consumer2.join();
    producer.join();
}

void TestThreadSafeQueHt()
{
    ThreadSafeQueueHt<MyClass> safe_que;

    std::thread consumer1(
            [&]()
            {
                for(;;)
                {
                    std::shared_ptr<MyClass> data = safe_que.wait_and_pop();
                    PrintMyClass("consumer1", data);
                }
            }
    );

    std::thread consumer2(
            [&]()
            {
                for(;;)
                {
                    std::shared_ptr<MyClass> data = safe_que.wait_and_pop();
                    PrintMyClass("consumer2", data);
                }
            }
    );

    std::thread producer(
            [&]()
            {
                for(int i = 0; i < 100; i++)
                {
                    MyClass mc(i);
                    safe_que.push(std::move(mc));
                }
            }
    );

    consumer1.join();
    consumer2.join();
    producer.join();
}
#endif

void TestRingLogSingleThread()
{
    LOG_INIT("log", "myname", 3);
    for (int i = 0;i < 1e6; ++i)
    {
        LOG_ERROR("my number is number my number is my number is my number is my number is my number is my number is %d", i);
    }
}

int64_t get_current_millis(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void* thdo(void* args)
{
    for (int i = 0;i < 1e7; ++i)
    {
        LOG_ERROR("my number is number my number is my number is my number is my number is my number is my number is %d", i);
    }
}

void TestRingLogMultiThread()
{
    LOG_INIT("log", "myname", 3);
    pthread_t tids[5];
    for (int i = 0;i < 5; ++i)
        pthread_create(&tids[i], NULL, thdo, NULL);

    for (int i = 0;i < 5; ++i)
        pthread_join(tids[i], NULL);
}

using namespace json11;

void testJsonParser()
{
    // R"(...)" 之间的内容会被直接当作字符串处理，不会对其中的特殊字符进行转义处理。这样可以方便地表示包含复杂内容的字符串。
    const string simple_test =
            R"({"k1":"v1", "k2":42, "k3":["a",123,true,false,null]})";
    string err;
    const auto json = Json::parse(simple_test, err);

    std::cout << "ttl: " << json["ttl"].int_value() << "\n";
    std::cout << "data: " << json["data"].dump() << "\n";

    for (auto &k : json["data"].array_items()) {
        std::cout << "    - " << k.dump() << "\n";
    }
}
int main()
{
//    TestThreadSafeStack();
//    TestThreadSafeQue();
//    TestThreadSafeQueHt();
//    TestLockFreeQue();
//    TestThreadPool();
//    TestRingLogSingleThread();
//    TestRingLogMultiThread();

    testJsonParser();

    return 0;
}
