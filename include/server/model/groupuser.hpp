#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"

// 群组用户，多了一个role角色信息，从User类直接继承，复用User的其它信息
class GroupUser : public User
{
public:
    void setRole(string role) { 
        this->role = role; 
    }

    string getRole() { 
        return this->role; 
    }

private:
    string role;        //组内角色  管理员(creator)/普通成员(normal)
};

#endif