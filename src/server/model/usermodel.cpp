#include "usermodel.hpp"
#include "server_database.hpp"
#include <iostream>
using namespace std;

//User表的增加方法
bool UserModel::insert(User &user)
{
    //1.组装sql语句
    // char sql[1024] = {0};
    // sprintf(sql, "insert into user(name, password, state) values('%s, '%s, '%s)",
    //             user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    string sql = "insert into user(name,password,state) VALUES('" + 
                                                            user.getName() + "','" + 
                                                            user.getPwd() + "','" + 
                                                            user.getState() + "')";

    //2.连接数据库，并更新数据
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql)){
            //获取插入成功的用户数据生成的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));

            return true;
        }
    }

    return false;
}

 //根据用户号码查询用户信息
User UserModel::query(int id)
{
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);

    //2.连接数据库，并更新数据
    MySQL mysql;
    if (mysql.connect())
    {
        //通过此sql语句在数据库中查询用户
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);       //获取用户行数据

            //组装用户信息
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);

                //释放资源,防止内存泄漏
                mysql_free_result(res);

                return user;            //返回用户信息
            }
        }
    }

    return User();      //不存在则返回默认构造的用户，其id = -1
}

//更新用户的状态信息
bool UserModel::updateState(User user)
{
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());

    //2.连接数据库，并更新数据
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql)){
            return true;
        }
    }

    return false;
}

//重置用户的状态信息
void UserModel::resetState()
{
    //1.组装sql语句
    char sql[1024] = "update user set state = 'offline' where state = 'online'";


    //2.连接数据库，并更新数据
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}