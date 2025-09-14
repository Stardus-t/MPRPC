#include <iostream>
#include <string>
#include "../user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

using namespace fixbug;

class UserService:public UserServiceRPC{//RPC服务发布端
public:
    bool Login(std::string name,std::string pwd){
        std::cout<<"Doing Local Service Login"<<std::endl;
        std::cout<<"Name: "<<name<<" Pwd: "<<pwd<<std::endl;
        return true;
    }

    bool Register(uint32_t id,std::string name,std::string pwd){
        std::cout<<"Doing Local Service Register"<<std::endl;
        std::cout<<"id: "<<id<<" Name: "<<name<<" Pwd: "<<pwd<<std::endl;
        return true;
    }

    //重写基类UserServiceRPC的虚函数
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,//这是用户发来的请求数据
                       ::fixbug::LoginResponse* response,//需要返回的数据
                       ::google::protobuf::Closure* done){

        //获取响应数据
        std::string name=request->name();
        std::string pwd=request->pwd();
        
        //做本地业务
        bool login_result=Login(name,pwd);

        //把响应写入（成功/错误）
        fixbug::ResultCode *code=response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);
    
        //执行回调操作(执行响应对象数据的序列化和网络发送（由框架完成）)
        done->Run();
    }
    void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,//这是用户发来的请求数据
                       ::fixbug::RegisterResponse* response,//需要返回的数据
                       ::google::protobuf::Closure* done){

        uint32_t id=request->id();
        std::string name=request->name();
        std::string pwd=request->pwd();
        
        bool ret=Register(id,name,pwd);
        //执行成功的响应
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);
        done->Run();
    }
};


int main(int argc,char **argv){
    //框架初始化（加载配置文件）
    MprpcApplication::Init(argc,argv);
    //UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());
    //启动一个rpc服务发布节点
    provider.Run();

    
    // UserService us;
    // us.Login("xxx","xxxx");
    return 0;
}