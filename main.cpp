#include<iostream>
using namespace std;

int main()
{
    cout << "Hello World!" << endl;
    time_t now = time(nullptr);
    tm* curr_tm = localtime(&now);  // 返回的结构体存储位置未知，不知何时释放，因此推荐使用安全版本。
    char time[80] = {0};
    strftime(time, 80, "%Y-%m-%d %H:%M:%S", curr_tm);
    cout << time << endl;
    return 0;
}