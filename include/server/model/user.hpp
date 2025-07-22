#ifndef USER_H
#define USER_H

#include <string>
using namespace std;


//匹配User表的ORM类   ->   不用见到具体的sql
//业务层 和 数据库 之间的ORM层
class User
{
public:
    //构造函数
    User(int id = -1, string name = "", string pwd = "", string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->password = pwd;
        this->state = state;
    }

    //设置表中用户信息
    void setId(int id){
        this->id = id;
    }

    void setName(string name){
        this->name = name;
    }

    void setPwd(string pwd){
        this->password = pwd;
    }

    void setState(string state){
        this->state = state;
    }

    //获取表中用户信息
    int getId(){
        return this->id;
    }

    string getName(){
        return this->name;
    }

    string getPwd(){
        return this->password;
    }

    string getState(){
        return this->state;
    }


protected:
    int id;
    string name;
    string password;
    string state;
};



#endif