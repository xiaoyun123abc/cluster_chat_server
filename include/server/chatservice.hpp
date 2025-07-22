#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>

#include <unordered_map>
#include <functional>
#include <mutex>
using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "json.hpp"
using json = nlohmann::json;

#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

//表示处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

//聊天服务器业务类
class ChatService
{
public:
    //获取单例对象的接口函数
    static ChatService* instance();

public:
    //处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 加入群组业务
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 群组聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //处理注销业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //获取消息对应的处理器
    MsgHandler getHandler(int msgid);

    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);

    //服务器异常，业务重置方法
    void reset();

    // 从redis消息队列中获取订阅的消息
    void handlerRedisSubscribeMessage(int userid, string msg);

private:
    //构造函数初始化
    ChatService();

private:
    //存储消息ID及其对应的业务处理方法
    unordered_map<int, MsgHandler> MsgHandlerMap_;

    //存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> userConnMap_;

    //定义互斥锁，保证userConnMap_的线程安全
    mutex connMutex_;

    
    /*数据库操作类对象，用于处理数据库中不同表，实现不同的功能*/
    //数据操作类对象
    UserModel userModel_;

    //离线消息数据操作类对象
    OfflineMsgModel offlineMsgModel_;

    //好友操作类对象
    FriendModel friendModel_;

    //群组操作类对象
    GroupModel groupModel_;

    //redis操作对象
    Redis redis_;
};

#endif