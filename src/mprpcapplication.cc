#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

MprpcConfig MprpcApplication::m_config;//MprpcConfig类型的MprpcApplication类的成员变量

void ShowArgsHelp(){
    std::cout<<"command -i <configfile>"<<std::endl;
}

void MprpcApplication::Init(int argc,char **argv){
    if(argc<2){//未传入任何参数，退出程序
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c=0;
    std::string config_file;
    while((c=getopt(argc,argv,"i:"))!=-1){//getopt解析命令。当命令中存在“-i”(表示命令附带参数)时，返回i。当命令解析完了也没有"-i"则返回-1
        switch (c)
        {
            case 'i'://程序接收“-i”选项
                config_file=optarg;//config_file等于命令行传入的文件
                break;
            case '?'://未定义选项
                std::cout<<"invalid args!"<<std::endl;
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            case ':'://没给参数
                std::cout<<"need <configfile>!"<<std::endl;
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }
    //开始加载配置文件
    m_config.LoadConfigfile(config_file.c_str());
    std::cout<<"rpcserverip: "<<m_config.Load("rpcserverip")<<std::endl;
    std::cout<<"rpcserverport: "<<m_config.Load("rpcserverport")<<std::endl;
    std::cout<<"zookeeperip: "<<m_config.Load("zookeeperip")<<std::endl;
    std::cout<<"zookeeperport: "<<m_config.Load("zookeeperport")<<std::endl;
}

MprpcApplication& MprpcApplication::GetInstance(){
    static MprpcApplication app;
    return app;
}

MprpcConfig& MprpcApplication::GetConfig(){
    return m_config;
}