#include <iostream>
#include <cstdlib>
#include <ctime>
#include "Singleton.h"
#include "Semaphore.h"
#include "ThreadSafeQue.h"
#include "ThreadSafeStack.h"

using namespace std;
#if 0 // 测试 线程安全的栈的并发操作，两个线程取，一个线程生产
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
#endif

int main()
{
//    TestThreadSafeStack();
    return 0;
}