#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include <vector>
#include "logger.h"

class FriendService:public RPC::FriendServiceRPC{
public:
    std::vector<std::string>GetFriendList(uint32_t userid){
        std::cout<<"do GetFriendList service! userid: "<<userid<<std::endl;
        std::vector<std::string> vec;
        vec.push_back("gao yang");
        vec.push_back("liu hong");
        vec.push_back("wang shuo");
        return vec;
    }
    void GetFriendList(::google::protobuf::RpcController* controller,
                       const ::RPC::GetFriendListRequest* request,
                       ::RPC::GetFriendListResponse* response,
                       ::google::protobuf::Closure* done){
        uint32_t userid=request->userid();
        std::vector<std::string> friendList=GetFriendList(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg(""); 
        for (std::string &name:friendList){
            std::string *p=response->add_friends();
            *p=name;
        }        
        std::cout << "响应中最终好友数量: " << response->friends_size() << std::endl; 
        for(int i=0;i<response->friends_size();++i) std::cout<<"index: "<<(i+1)<<" name: "<<response->friends(i)<<std::endl;     
        done->Run();
    }
};

int main(int argc,char **argv){
    LOG_INFO("first log message!");
    LOG_ERR("%s:%s%d",__FILE__,__FUNCTION__,__LINE__);
    //框架初始化（加载配置文件）
    MprpcApplication::Init(argc,argv);
    //UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());
    //启动一个rpc服务发布节点
    provider.Run();

    // UserService us;
    // us.Login("xxx","xxxx");
    return 0;
}