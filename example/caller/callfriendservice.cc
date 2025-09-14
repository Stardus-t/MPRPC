#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"
#include "mprpccontroller.h"


int main(int argc,char **argv){
    //使用mrpc框架享受rpc服务调用
    //初始化框架
    MprpcApplication::Init(argc,argv);

    //调用rpc方法Login
    RPC::FriendServiceRPC_Stub stub(new MprpcChannel());
    //rpc方法的请求参数
    RPC::GetFriendListRequest request;
    request.set_userid(uint32_t(2000));
    std::cout << "客户端发送请求：userid = " << request.userid() << std::endl;
    //rpc方法的响应
    RPC::GetFriendListResponse response;
    std::cout << "响应对象初始状态：friends_size = " << response.friends_size() 
              << ", errcode = " << response.result().errcode() << std::endl;
    //发起rpc方法的调用
        //构建MprpcController实例
    MprpcController controller;
    stub.GetFriendList(&controller,&request,&response,nullptr);//RPCChannel::callMethod
    //rpc调用完成
    if(controller.Failed()){
        //发生错误
        std::cout<<controller.ErrorText()<<std::endl;
    }
    else{
        if(0==response.result().errcode()){
            std::cout<<"rpc GetFriendList success! "<<std::endl;
            int size=response.friends_size();
            std::cout<<size<<std::endl;
            std::cout << "接收到的响应中好友数量: " << response.friends_size() << std::endl;
            for(int i=0;i<size;++i) std::cout<<"index: "<<(i+1)<<" name: "<<response.friends(i)<<std::endl;
        }
        else std::cout<<"rpc GetFriendLists response error: "<<response.result().errcode()<<std::endl;
    }
    

    return 0;
}