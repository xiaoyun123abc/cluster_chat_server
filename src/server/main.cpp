#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
using namespace std;

//处理服务器ctrl_c结束后，重置user的状态信息
void resetHandler(int)                              //回调函数
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char **argv)
{
    // 检查命令行参数数量
    if (argc < 3) {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000 or 6002" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, resetHandler);                   // 服务器如果异常下线，则将所有用户的状态设置为offline

    EventLoop loop;
    InetAddress addr(ip, port);                     //服务器IP+port
    ChatServer server(&loop, addr, "ChatServer");   //定义服务器对象

    server.start();                                 //启动服务器，监听连接请求
    loop.loop();                                    //事件循环

    return 0;
}