/*
muduo网络库给用户提供了两个主要的类：
    TcpServer:用于编写服务器程序的
    TcpClient:用于编写客户端程序的

epoll+线程池
好处：能够将网络I/O的代码和业务代码区分开
                              |
                      (1)用户的连接和断开 
                      (2)用户的可读写事件
    只需要关注这两件事如何做即可，至于什么时候发生，由网络库监听
*/
#include <iostream>
#include <string>
using namespace std;

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;

#include <functional>   //要用绑定器

/*
基于muduo网络库开发服务器程序
1.组合TcpServer对象
2.创建EventLoop事件循环对象的指针
3.明确TcpServer构造函数需要什么参数，输出ChatServer的构造函数
4.在当前服务器类的构造函数当中，注册处理连接的回调函数和处理读写事件的回调函数
5.设置合适的服务端线程数量,muduo库会自己分配I/O线程和worker线程
*/
class ChatServer
{
public:
    ChatServer(EventLoop* loop,                 //事件循环，也可以理解成reactor反应堆
               const InetAddress& listenAddr,   //IP+Port
               const string& nameArg)           //服务器名称
        :server_(loop, listenAddr, nameArg)
        ,loop_(loop)
    {
        //给服务器注册用户连接的创建和断开回调
        //当底层监听到有用户的连接和断开，就调用回调进行执行
        server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));   //_1:参数占位符

        //给服务器注册用户读写事件回调
        server_.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

        //设置服务器端的线程数量  1个I/O线程，3个worker线程
        server_.setThreadNum(4);
    }

    //开启事件循环
    void start()
    {
        server_.start();
    }

//主要精力可以放在写以下两个函数上
private:
    //专门处理用户的连接和断开    epoll   listenfd   accept  -> muduo库都封装了，只暴露了回调接口
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            cout << conn->peerAddress().toIpPort() << " -> " 
                 << conn->localAddress().toIpPort() << " state:online" << endl;
        }
        else
        {
            cout << conn->peerAddress().toIpPort() << " -> " 
                 << conn->localAddress().toIpPort() << " state:offline" << endl;
            conn->shutdown();    //相当于close(fd);
            
            // loop_->quit();       
        }
    }

    //专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr &conn,   //连接
                   Buffer *buffer,                 //缓冲区
                   Timestamp time)                 //接收到数据的事件信息
    {
        string buf = buffer->retrieveAllAsString();
        cout << "recv data:" << buf << "time:" << time.toString() << endl;
        conn->send(buf);
    }

    muduo::net::TcpServer server_;
    muduo::net::EventLoop *loop_;
};

int main()
{
    EventLoop loop;     //epoll
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();     //启动服务器  listenfd epoll_ctl -> epoll
    loop.loop();        //epoll_wait以阻塞方式等待新用户连接，已连接用户的读写事件等

    return 0;
}
