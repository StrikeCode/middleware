//
// Created by Administrator on 2025/3/4.
//

#include <iostream>
using namespace std;
#include "../include/Connection.h"
#include "../include/CommonConnectionPool.h"
#define WITH_CONN_POOL 0



int main()
{
    // 测无连接池且多线程为什么要如下代码？？？
    Connection conn;
    conn.connect("127.0.0.1", 3306, "root", "root", "chat");

    /*Connection conn;
    char sql[1024] = { 0 };
    sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
        "zhang san", 20, "male");
    conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
    conn.update(sql);*/

    clock_t begin = clock();

    thread t1([]() {
#if WITH_CONN_POOL
        ConnectionPool *cp = ConnectionPool::getConnectionPool();
#endif
        for (int i = 0; i < 2500; ++i)
        {
#if WITH_CONN_POOL
            char sql[1024] = { 0 };
            sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
                "zhang san", 20, "male");
            shared_ptr<Connection> sp = cp->getConnection();
            sp->update(sql);
#else
            Connection conn;
            char sql[1024] = { 0 };
            sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
                    "zhang san", 20, "male");
            conn.connect("127.0.0.1", 3306, "root", "root", "chat");
            conn.update(sql);
#endif
        }
    });
    thread t2([]() {
#if WITH_CONN_POOL
        ConnectionPool *cp = ConnectionPool::getConnectionPool();
#endif
        for (int i = 0; i < 2500; ++i)
        {
#if WITH_CONN_POOL
            char sql[1024] = { 0 };
            sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
                "zhang san", 20, "male");
            shared_ptr<Connection> sp = cp->getConnection();
            sp->update(sql);
#else
            Connection conn;
            char sql[1024] = { 0 };
            sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
                    "zhang san", 20, "male");
            conn.connect("127.0.0.1", 3306, "root", "root", "chat");
            conn.update(sql);
#endif
        }
    });
    thread t3([]() {
#if WITH_CONN_POOL
        ConnectionPool *cp = ConnectionPool::getConnectionPool();
#endif
        for (int i = 0; i < 2500; ++i)
        {
#if WITH_CONN_POOL
            char sql[1024] = { 0 };
            sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
                "zhang san", 20, "male");
            shared_ptr<Connection> sp = cp->getConnection();
            sp->update(sql);
#else
            Connection conn;
            char sql[1024] = { 0 };
            sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
                    "zhang san", 20, "male");
            conn.connect("127.0.0.1", 3306, "root", "root", "chat");
            conn.update(sql);
#endif
        }
    });
    thread t4([]() {
#if WITH_CONN_POOL
        ConnectionPool *cp = ConnectionPool::getConnectionPool();
#endif
        for (int i = 0; i < 2500; ++i)
        {
#if WITH_CONN_POOL
            char sql[1024] = { 0 };
            sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
                "zhang san", 20, "male");
            shared_ptr<Connection> sp = cp->getConnection();
            sp->update(sql);
#else
            Connection conn;
            char sql[1024] = { 0 };
            sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
                    "zhang san", 20, "male");
            conn.connect("127.0.0.1", 3306, "root", "root", "chat");
            conn.update(sql);
#endif
        }
    });

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    clock_t end = clock();
    cout << (double)(end - begin) * 1000 / CLOCKS_PER_SEC << "ms" << endl;


#if 0
    for (int i = 0; i < 10000; ++i)
	{
		// 不能放外面，放外面相当于1个人对1个页面做10000次请求
		// 我们的要求是 10000个人都做1次请求
		Connection conn;
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
			"zhang san", 20, "male");
		conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
		conn.update(sql);

		/*shared_ptr<Connection> sp = cp->getConnection();
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
			"zhang san", 20, "male");
		sp->update(sql);*/
	}
#endif

    return 0;
}

