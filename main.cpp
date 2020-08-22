#include "config.h"
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
    //需要修改的数据库信息,登录名,密码,库名
    string user = "root";
    string passwd = "root";
    string databasename = "SDB";

    //命令行解析
    Config config;
    config.parse_arg(argc, argv);

    WebServer server;

    //初始化
    server.init(config.PORT, user, passwd, databasename, config.LOGWrite, 
                config.OPT_LINGER, config.TRIGMode,  config.sql_num,  config.thread_num, 
                config.close_log, config.actor_model);
    

    //日志
    server.log_write();
    std::cout<<"log init finish"<<endl;
    //数据库
    server.sql_pool();
    std::cout<<"sql_pool init finish"<<endl;

    //线程池
    server.thread_pool();
    std::cout<<"thread_pool init finish"<<endl;

    //触发模式
    server.trig_mode();
    std::cout<<"trig_model init finish"<<endl;

    //监听
    server.eventListen();
    std::cout<<"eventloop init finish"<<endl;
    std::cout<<"running..."<<endl;
    //运行
    server.eventLoop();


    return 0;
}
