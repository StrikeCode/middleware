#include "../include/Timer.h"
#include <iostream>
using namespace std::chrono;

int main()
{
    CppTime::Timer t;
    t.add(seconds(2), [](CppTime::timer_id) { std::cout << "oneshot Timer!\n"; });
    std::this_thread::sleep_for(seconds(3));

    CppTime::Timer t1;
    auto id = t1.add(seconds(1), [](CppTime::timer_id) { std::cout << "periodic timer !\n"; }, seconds(1));
    std::this_thread::sleep_for(seconds(10));
    t.remove(id);
    return 0;
}