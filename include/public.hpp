#ifndef PUBLIC_H
#define PUBLIC_H

/*
server和client的公共文件
*/
enum EnMsgType
{
    LOGIN_MSG = 1,          //登录消息
    LOGIN_MSG_ACK = 2,      //登录响应消息
    REG_MSG = 3,            //注册消息
    REG_MSG_ACK = 4,        //注册响应消息
    ONE_CHAT_MSG = 5,       //聊天消息
    ADD_FRIEND_MSG = 6,     //添加好友消息

    CREATE_GROUP_MSG = 7,   //创建群组
    ADD_GROUP_MSG = 8,      //加入群组
    GROUP_CHAT_MSG = 9,     //群聊天

    LOGIN_OUT_MSG = 10,     //注销消息
};

/*
登录消息:(由客户端发送，服务器解析)
    {"msgid":LOGIN_MSG, "id":用户id, "password":用户密码}


登录响应消息:(由服务器解析客户端发送的登录消息后，返回的应答信息，客户端需要解析这些应答信息)
    (1)已经在线
            {"msgid":LOGIN_MSG_ACK, "errno":2, "errmsg":"this count is using, input another!"}

    (2)成功登录
            {"msgid":LOGIN_MSG_ACK, "error":0, "id":用户Id, "name":用户名, "offlinemessage":vec1, "friends": vec2, "groups":groupV}

            vec1:vector<string> : 数组中每一个string都是一条离线消息
            vec2:vector<string> : 数组中每一个string都是一个用户信息的json序列化字符
            groupV:vector<string> : 每一个string都是一个群的json序列化字符，此序列化字符中还有vector<string> userV，是此群中所有用户的序列化字符数组
            
    (3)登录失败
            {"msgid":LOGIN_MSG_ACK, "errno":0, "errmsg":"id or password error!"}


注册消息:(由客户端发送，服务器解析)
    {"msgid":REG_MSG, "name":用户名, "password":用户密码}


注册响应消息:(由服务器解析客户端发送的登录消息后，返回的应答信息，客户端需要解析这些应答信息)
    (1)注册成功
            {"msgid":REG_MSG_ACK, "error":0, "id":用户id}
    (2)注册失败
            {"msgid":REG_MSG_ACK, "errno":1, "errmsg":"register error"}


一对一聊天消息:
    {"msgid":ONE_CHAT_MSG, "id":用户id, "name":用户名, "to":聊天对象的id, "msg":信息}





*/


#endif