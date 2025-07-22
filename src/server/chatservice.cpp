#include "chatservice.hpp"
#include "public.hpp"
#include <friendmodel.hpp>

#include <muduo/base/Logging.h>
#include <vector>
#include <map>
using namespace muduo;
using namespace std;

//获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

//注册消息以及对应的Handler回调函数
//将消息和事件处理器绑定
ChatService::ChatService()
{
    MsgHandlerMap_.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    MsgHandlerMap_.insert({LOGIN_OUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    MsgHandlerMap_.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    MsgHandlerMap_.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    MsgHandlerMap_.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    //群组业务管理相关事件处理回调注册
    MsgHandlerMap_.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    MsgHandlerMap_.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    MsgHandlerMap_.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    //连接redis服务器
    if (redis_.connect())
    {
        //设置上报消息的回调
        redis_.init_notify_handler(std::bind(&ChatService::handlerRedisSubscribeMessage, this, _1, _2));
    }
}

//获取消息对应的处理器   (从回调函数表中查询对应的是哪个回调函数)
MsgHandler ChatService::getHandler(int msgid)
{
    //记录错误日志，msgid没有对应的事件处理回调
    auto it = MsgHandlerMap_.find(msgid);

    //判断msgid对应的回调函数是否存在
    if (it == MsgHandlerMap_.end())
    {
        //不存在返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp){
            LOG_ERROR << "msgid:" << msgid << "can not find handler!";
        };
    }
    else
    {
        //存在则返回对应的处理器
        return MsgHandlerMap_[msgid];
    }
}

//处理登录业务   
//ORM:对象关系映射  ->  在业务层操作的都是对象   ->  业务代码和数据库代码解耦合
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    /*
    conn:与客户端的TCP连接，其中封装了客户端的通信套接字
    js:客户端发送的序列化之后的json字符串
    time:时间戳
    */
    int id = js["id"].get<int>();       //用户id
    string pwd = js["password"];        //用户密码

    //在用户表中查询此id对应的用户是否存在或者是否已经在线
    User user = userModel_.query(id);   
    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            //该用户已经登录，不允许重复登录
            //构造应答ACK：{"msgid":LOGIN_MSG_ACK, "errno":, "errmsg":错误信息}
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;              
            response["errmsg"] = "this count is using, input another!";
            conn->send(response.dump());
        } 
        else
        {
            //登录成功
            //(1)记录用户连接信息 -> 但要考虑线程安全问题(因为onMessage就是在不同的工作线程中被回调的，onMessage内所有代码都是在多线程下操作的)
            //大括号代表作用域，离开大括号互斥锁就解锁了
            {
                //将此用户id和对应的连接conn加入到userConnMap_字典中
                lock_guard<mutex> lock(connMutex_);
                userConnMap_.insert({id, conn});
            }

            //id用户登录成功后，向redis订阅channel(id)
            redis_.subscribe(id);

            //(2)更新用户状态信息   state  offline => online
            user.setState("online");
            userModel_.updateState(user);

            //(3)发生连接成功ACK信息
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;                      //成功则为0
            response["id"] = user.getId();
            response["name"] = user.getName();

            //查询该用户是否有离线消息
            vector<string> vec1 = offlineMsgModel_.query(id);
            if (!vec1.empty())
            {
                response["offlinemsg"] = vec1;

                //读取该用户的离线消息后，把该用户的所有离线消息删除掉
                offlineMsgModel_.remove(id);
            }

            //查询该用户的好友信息并返回
            /*
            
            */
            vector<User> userVec = friendModel_.query(id);
            if (!userVec.empty())
            {
                vector<string> vec2;
                for (User &user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 查询用户的群组信息
            vector<Group> groupuserVec = groupModel_.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[[groupid:[xxx, xxx, xxx, xxx]]]
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json gropjson;
                    gropjson["id"] = group.getId();
                    gropjson["groupname"] = group.getName();
                    gropjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole(); 
                        userV.push_back(js.dump());
                    }
                    gropjson["users"] = userV;
                    groupV.push_back(gropjson.dump());
                }

                response["groups"] = groupV;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        //该用户不存在/用户存在但是密码错误，登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;                          //失败则为1
        response["errmsg"] = "id or password error!";
        conn->send(response.dump());
    }

}

//处理注册业务  name password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = userModel_.insert(user);
    if (state){
        //注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;              //成功则为0
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else{
        //注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;              //失败则为1
        response["errmsg"] = "register error";
        conn->send(response.dump());
    }
}

//处理客户端异常退出
//为什么客户端主动退出时可以改动，而异常退出时信息没有改动呢？
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(connMutex_);
        for (auto it = userConnMap_.begin(); it != userConnMap_.end(); ++it)
        {
            if (it->second == conn)
            {
                //从map表删除用户的连接信息
                user.setId(it->first);
                userConnMap_.erase(it);
                break;
            }
        }
    }

    //用户注销，相当于就是下线，在redis中取消订阅通道
    redis_.unsubscribe(user.getId());

    //更新用户的状态信息
    if (user.getId() != -1)
    {
        user.setState("offline");
        userModel_.updateState(user);
    }
}

//服务器异常，业务重置方法
void ChatService::reset()
{
    //把online状态的用户，设置成offline
    userModel_.resetState();
}

//一对一聊天业务
/*
客户端和服务器要约定好msgid，以便使用序列化和反序列化
    id:1
    name:"zhaotianyu"
    to:3
    msg:"xxxxxxx"
*/
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    //获取申请聊天的对象的id
    int toid = js["toid"].get<int>();

    {
        //操作连接表  ->  设置互斥锁保证线程安全
        lock_guard<mutex> lock(connMutex_);

        /*(1)查询要聊天的用户是否在同一台服务器上进行注册*/
        //从连接表中查询此用户是否在线  ->  在线则直接将消息推送过去
        auto it = userConnMap_.find(toid);
        if (it != userConnMap_.end())
        {
            //toid在线，转发消息  ->  服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }

    //(2)查询toid是否在线
    //如果在线，说明没有在同一台服务器上登录  ->  通过redis发布消息
    User user = userModel_.query(toid);
    if (user.getState() == "online")
    {
        redis_.publish(toid, js.dump());
        return;
    }

    //toid不在线，存储离线消息  ->  存储在申请聊天的对象的离线消息表中
    //使用存储离线消息的MySQL表的操作对象offlineMsgModel_进行处理
    offlineMsgModel_.insert(toid, js.dump());
}

//添加好友业务   msgid  id  friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    //存储好友信息
    //使用对应数据库的操作对象
    friendModel_.insert(userid, friendid);
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];      //组名
    string desc = js["groupdesc"];      //组功能

    // 存储新创建的群组信息
    Group group(-1, name, desc);        //组id在创建时赋值
    if (groupModel_.createGroup(group))
    {
        // 存储群组创建人信息
        groupModel_.addGroup(userid, group.getId(), "creator");
    }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    groupModel_.addGroup(userid, groupid, "normal");
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    //获取groupId组内除userid自己，其它所有的组员id数组
    vector<int> useridVec = groupModel_.queryGroupUsers(userid, groupid);
    
    lock_guard<mutex> lock(connMutex_);     //保证map表操作的线程安全问题
    //对每一个组员
    for (int id : useridVec)
    {
        auto it = userConnMap_.find(id);    
        if (it != userConnMap_.end())
        {
            // 在线且在同一台服务器上 -> 转发群消息
            it->second->send(js.dump());
        }
        else
        {
            // 不在同一台服务器 -->  查询是否在线  -->  在线则发布消息到redis上相应通道
            User user = userModel_.query(id);
            if (user.getState() == "online")
            {
                redis_.publish(id, js.dump());      //发布消息
            }
            else
            {
                // 离线 -> 存储离线群消息
                offlineMsgModel_.insert(id, js.dump());
            }
        }
    }
}


//处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    //限制锁的范围
    {
        //加锁
        lock_guard<mutex> lock(connMutex_);

        //将用户从已连接表中删除
        auto it = userConnMap_.find(userid);
        if (it != userConnMap_.end())
        {
            userConnMap_.erase(it);
        }
    }

    //用户注销，相当于下线，再redis中取消订阅通道
    redis_.unsubscribe(userid);

    //更新用户的状态信息
    User user(userid, "", "offline");
    userModel_.updateState(user);
}


// 从redis消息队列中获取订阅的消息
void ChatService::handlerRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(connMutex_);
    auto it = userConnMap_.find(userid);
    if (it != userConnMap_.end())
    {
        it->second->send(msg);
        return;
    }

    //存储该用户的离线消息
    offlineMsgModel_.insert(userid, msg);
}
