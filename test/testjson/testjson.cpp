#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;

//json序列化示例1
string func1()
{
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhangsan";
    js["to"] = "lisi";
    js["msg"] = "hello, what are you do?";

    return js.dump();
}

//json序列化示例2
string func2()
{
    //先定义一个json对象
    json js;
    // 添加数组
    js["id"] = {1,2,3,4,5};

    // 添加key-value
    js["name"] = "zhang san";

    // 添加对象
    js["msg"]["zhang san"] = "hello world";
    js["msg"]["liu shuo"] = "hello china";

    // 上面等同于下面这句一次性添加数组对象
    js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello china"}};

    return js.dump();
}

//json序列化示例代码3
string func3()
{
    json js;
    
    //直接序列化一个vector容器
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);
    js["list"] = vec;

    //直接序列化一个map容器
    map<int , string> m;
    m.insert({1, "黄山"});
    m.insert({2, "华山"});
    m.insert({3, "泰山"});
    js["path"] = m;

    return js.dump();
}

int main()
{
    //普通数据的反序列化  => json字符串反序列化为数据对象(看作容器，方便访问)
    string recvBuf = func1();
    json jsbuf = json::parse(recvBuf);
    cout<<jsbuf["msg_type"]<<endl;
    cout<<jsbuf["from"]<<endl;
    cout<<jsbuf["to"]<<endl;
    cout<<jsbuf["msg"]<<endl;
    cout<<"==============================="<<endl;

    //数组数据的反序列化
    recvBuf = func2();
    jsbuf = json::parse(recvBuf);
    auto arr = jsbuf["id"];
    cout<<arr[2]<<endl;

    auto msgjs = jsbuf["msg"];
    cout<<msgjs["zhang san"]<<endl;
    cout<<"==============================="<<endl;

    //vector及map容器的反序列化
    recvBuf = func3();
    jsbuf = json::parse(recvBuf);
    vector<int> vec = jsbuf["list"];     //js对象中的数组类型，直接放入vector容器当中
    for (int &v : vec){
        cout<<v<<endl;
    }

    map<int, string> mymap = jsbuf["path"];
    for (auto &p : mymap){
        cout<<p.first<<" "<<p.second<<endl;
    }

    return 0;
}