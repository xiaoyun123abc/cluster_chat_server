#ifndef SERVER_DATABASE_H
#define SERVER_DATABASE_H

#include <mysql/mysql.h>
#include <string>
using namespace std;


// 数据库操作类
class MySQL
{
public:
    // 初始化数据库连接
    MySQL();

    // 释放数据库连接资源
    ~MySQL();

    // 连接数据库
    bool connect();

    // 更新操作
    bool update(string sql);
    
    // 查询操作
    MYSQL_RES* query(string sql);

    //获取连接
    MYSQL* getConnection();

private:
    MYSQL* _conn;  // MySQL连接对象(相当于与MySQL数据库服务器的一条连接)
};


#endif