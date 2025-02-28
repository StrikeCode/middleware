//
// Created by 江宇豪 on 2025/2/28.
//
#include "../include/ObjectPool.h"
int main()
{
    // 在ObjectPool.h 打开和关闭对new 和 delete重载进行测试
    // without ObjectPool 33.03ms 40.439ms 40.231ms
    // with ObjectPool 15.001ms 15.662ms 23.118ms
    clock_t startTime, endTime;
    startTime = clock();//计时开始
    Queue<int> que;
    for (int i = 0; i < 1000000; ++i)
    {
        que.push(i);
        que.pop();
    }
    endTime = clock();
    cout << "The run time is:" << (double)(endTime - startTime) / CLOCKS_PER_SEC * 1000 << "ms" << endl;

    return 0;
}