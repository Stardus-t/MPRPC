#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"

int main(int argc,char **argv){
    //使用mrpc框架享受rpc服务调用
    //初始化框架
    MprpcApplication::Init(argc,argv);

    //调用rpc方法Login
    fixbug::UserServiceRPC_Stub stub(new MprpcChannel());
    //rpc方法的请求参数
    fixbug::LoginRequest request;
    request.set_name("Todd");
    request.set_pwd("123456");
    //rpc方法的响应
    fixbug::LoginResponse response;
    //发起rpc方法的调用
    stub.Login(nullptr,&request,&response,nullptr);//RPCChannel::callMethod
    //rpc调用完成
    if(0==response.result().errcode()) std::cout<<"rpc login response: "<<response.success()<<std::endl;
    else std::cout<<"rpc login response error: "<<response.result().errcode()<<std::endl;

    //调用rpc方法register
    fixbug::RegisterRequest req;
    req.set_id(u_int32_t(2000));
    req.set_name("mprpc");
    req.set_pwd("666666");
    fixbug::RegisterResponse rsp;
    stub.Register(nullptr,&req,&rsp,nullptr);
    if(0==rsp.result().errcode()) std::cout<<"rpc register response: "<<rsp.success()<<std::endl;
    else std::cout<<"rpc register response error: "<<rsp.result().errcode()<<std::endl;
    return 0;
}