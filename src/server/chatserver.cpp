#include "chatserver.hpp"
#include "chatservice.hpp"
#include "public.hpp"

#include "json.hpp"
using json = nlohmann::json;

#include <functional>
#include <string>

using namespace std;
using namespace placeholders;


ChatServer::ChatServer(EventLoop* loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
        :server_(loop, listenAddr, nameArg)
        ,loop_(loop)
{
    //注册链接回调  =>  有连接请求时回调此函数
    server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

    //注册消息回调  =>  有读写请求时回调此函数
    server_.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    //设置线程数量
    server_.setThreadNum(4);
}

//启动服务器
void ChatServer::start()
{
    server_.start();
}

/*具体回调函数实现*/
//上报链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    /*
    conn代表与客户端的TCP连接
    一对一关系：每个 TcpConnection 对象对应一个 独立的客户端连接。
    内部封装：TcpConnection 内部通过 Socket 类管理客户端的套接字（文件描述符）
    */

    //检查用户(客户端)是否断开连接
    if (!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);   //用于处理客户端异常退出业务
        conn->shutdown();
    }
}

//上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
    //读取到的是客户端发送过来的经过json序列化后的数据
    string buf = buffer->retrieveAllAsString();

    //数据的反序列化
    json js = json::parse(buf);

    //目的：完全解耦网络模块的代码和业务模块的代码 -> 回调思想
    //通过js["msgid"]获取 -> 业务处理器handler，获知是哪种请求
    //ChatService::instance()  ->  获取聊天服务对象实例，这是静态的，所有对象共享
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());

    //回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn, js, time);
}