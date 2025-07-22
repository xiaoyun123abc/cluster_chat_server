#include "server_database.hpp"

#include <muduo/base/Logging.h>

// 数据库配置信息
static string server = "127.0.0.1";
static string user = "root";
static string password = "zty780130zz";
static string dbname = "chat";

// 初始化数据库连接
MySQL::MySQL()
{
    //开启资源空间
    _conn = mysql_init(nullptr);
}

// 释放数据库连接资源
MySQL::~MySQL()
{
    if (_conn != nullptr)
        mysql_close(_conn);
}

// 连接数据库
bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                                  password.c_str(), dbname.c_str(), 3306, nullptr, 0);

    //p不为空 -> 连接成功
    if (p != nullptr)
    {
        //相当于使得在代码上能够支持中文
        //因为C和C++默认都是ASCAII码，如果不设置，那么直接从数据库拉取中文，会在本地代码编程上显示问号
        mysql_query(_conn, "set names gbk");

        LOG_INFO << "connect mysql success!";  //提示连接成功
    }
    else
    {
        LOG_INFO << "connect mysql fail!";     //提示连接失败
    }

    return p;
}

// 更新操作
bool MySQL::update(string sql)
{
    //使用MySQL C API的mysql_query()函数执行SQL语句
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败!";
        return false;
    }
    return true;
}

// 查询操作
MYSQL_RES* MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败!";
        return nullptr;
    }
    return mysql_store_result(_conn);
}

//获取连接
MYSQL* MySQL::getConnection()
{
    return _conn;
}