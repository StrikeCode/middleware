#include<iostream>
#include <cstdlib>
#include <ctime>
#include "Singleton.h"
#include "Semaphore.h"
#include "ThreadSafeQue.h"
#include "ThreadSafeStack.h"

using namespace std;
void TestThreadSafeStack()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    ThreadSafeStack<int> stk;
    for(int i = 0; i < 10; ++i)
    {
        stk.push(std::rand() % 100);
    }
    int val;
    while(!stk.empty())
    {
        std::shared_ptr<int> pVal = stk.pop();
        std::cout << *pVal << " " << std::endl;
    }
}
int main()
{
    TestThreadSafeStack();
    return 0;
}