//
// Created by Administrator on 2025/3/4.
//
#include "../include/Connection.h"
#include "../include/RingLog.h"
#include <iostream>
using namespace std;

Connection::Connection()
{
    // 初始化数据库连接
    _conn = mysql_init(nullptr);
}

Connection::~Connection()
{
    // 释放数据库连接资源
    if (_conn != nullptr)
        mysql_close(_conn);
}

bool Connection::connect(string ip, unsigned short port,
                         string username, string password, string dbname)
{
    // 连接数据库
    MYSQL *p = mysql_real_connect(_conn, ip.c_str(), username.c_str(),
                                  password.c_str(), dbname.c_str(), port, nullptr, 0);
    return p != nullptr;
}

bool Connection::update(string sql)
{
    // 更新操作 insert、delete、update
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_ERROR("更新失败:%s",sql.c_str());
        return false;
    }
    return true;
}

MYSQL_RES* Connection::query(string sql)
{
    // 查询操作 select
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_ERROR("查询失败:%s", sql.c_str());
        return nullptr;
    }
    // 直接从服务器读取一个查询的结果而不是存储它到一个临时表或者一个客户端的缓存里面
    return mysql_use_result(_conn);
}