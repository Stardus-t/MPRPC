#include "rpcprovider.h"
#include "mprpcapplication.h"
#include <string>
#include <google/protobuf/descriptor.h>
#include <functional>
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"

void RpcProvider::NotifyService(google::protobuf::Service * service){
    ServiceInfo serviceinfo;
    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor*pservericeDesc=service->GetDescriptor();
    //获取服务的名字
    const std::string service_name=pservericeDesc->name();
    //获取服务对象service的方法数量
    int methodCnt=pservericeDesc->method_count();

    // std::cout<<"service_name: "<<service_name<<std::endl;
    LOG_INFO("service_name: %s",service_name.c_str());
    for(int i=0;i<methodCnt;i++){
        //获取服务对象的方法
        const google::protobuf::MethodDescriptor* pmethodDesc=pservericeDesc->method(i);
        std::string method_name=pmethodDesc->name();
        serviceinfo.m_MethodMap.insert({method_name,pmethodDesc});
        // std::cout<<"method_name: "<<method_name<<std::endl;
        LOG_INFO("method_name: %s",method_name.c_str());
    }
    serviceinfo.m_service=service;
    m_serviceMap.insert({service_name,serviceinfo});
}

void RpcProvider::Run(){
    std::string ip=MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port=atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
     //创建TcpServer
    muduo::net::InetAddress address(ip,port);//ip port ipv4=false
    muduo::net::TcpServer server(&m_eventloop,address,"RpcProvider");
    //绑定连接回调和消息读写回调方法
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this,std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    //设置muduo库线程数量
    server.setThreadNum(4);

    //把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    ZkClient zkCli;
    zkCli.Start();
    //service_name为永久性节点 method_name为临时性节点
    for (auto &sp:m_serviceMap){
        std::string service_path="/"+sp.first;
        zkCli.Create(service_path.c_str(),nullptr,0);
        for(auto &mp:sp.second.m_MethodMap){
            std::string method_path=service_path+"/"+mp.first;
            char method_path_data[128]={0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);
            zkCli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
    }

    std::cout<<"RPCProvider starts service at ip: "<<ip<<" port: "<<port<<std::endl;
    //启动网络服务
    server.start();
    m_eventloop.loop();//启动事件循环
}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn){
    //如果断开连接
    if(!conn->connected()) conn->shutdown();
}

//已建立连接用户的读写事件回调 如果远程有一个rpc服务的调用请求，那么OnMessage方法就会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn,muduo::net::Buffer *buffer,muduo::Timestamp){
    std::string recv_buf=buffer->retrieveAllAsString();//接收请求RPC方发送过来的数据
    /*数据格式
    header_size(4个字节)+header_str+args_str*/
    //从字符流中读取前4个字节的内容
    uint32_t header_size=0;
    recv_buf.copy((char*)&header_size,4,0);//从buffer中读取4个字节的内容放入header_size中（字符转成整数）
    //根据header_size读取原始数据头的字符流
    std::string rpc_header_str=recv_buf.substr(4,header_size);
    //反序列化数据，得到rpc请求的详细信息
    mprpc::RPCHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(rpc_header_str)){
        //数据头反序列化成功
        service_name=rpcHeader.service_name();
        method_name=rpcHeader.method_name();
        args_size=rpcHeader.args_size();

    }
    else{
        //数据头反序列化失败
        std::cout<<"rpcheader_str: "<<rpc_header_str<<"parse error!"<<std::endl;
        return ;
    }
    //获取rpc方法参数的字符流数据
    std::string args_str=recv_buf.substr(4+header_size,recv_buf.size());

    //打印调试信息
    std::cout<<"====================================================================="<<std::endl;
    std::cout<<"header_size: "<<header_size<<std::endl;
    std::cout<<"rpc_header_str: "<<rpc_header_str<<std::endl;
    std::cout<<"service_name: "<<service_name<<std::endl;
    std::cout<<"method_name: "<<method_name<<std::endl;
    std::cout<<"args_str: "<<args_str<<std::endl;
    std::cout<<"====================================================================="<<std::endl;

    //获取service对象和method 对象
    auto it=m_serviceMap.find(service_name);
    if(it==m_serviceMap.end()){
        std::cout<<service_name<<" is not exist"<<std::endl;
        return ;
    }
    google::protobuf::Service *service=it->second.m_service;//service对象
    auto mit=it->second.m_MethodMap.find(method_name);
    if(mit==it->second.m_MethodMap.end()){
        std::cout<<service_name<<": "<<method_name<<" is not exist"<<std::endl;
        return;
    }
    const google::protobuf::MethodDescriptor *method=mit->second; //method对象
    
    //生成rpc方法调用的请求request和响应response
    google::protobuf::Message *request=service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str)){
        std::cout<<"request parse error, content: "<<args_str<<std::endl;
        return ;
    }
    google::protobuf::Message *response=service->GetResponsePrototype(method).New();
    //给下面CallMethod方法的调用绑定一个Closure的回调函数
    google::protobuf::Closure*done=google::protobuf::NewCallback<RpcProvider,
                                                            const muduo::net::TcpConnectionPtr&,
                                                            google::protobuf::Message*>
                                                            (this,&RpcProvider::SendRpcResponse,conn,response);
    //在框架上根据远端rpc请求，调用当前节点上发布的方法
    service->CallMethod(method,nullptr,request,response,done);
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn,google::protobuf::Message*response){
    std::string response_str;
    //response序列化,序列化成功后，用网络将rpc方法执行结果发送回rpc的调用方
    if(response->SerializeToString(&response_str)){
        std::cout<<"sended message: "<<response_str<<std::endl;
        conn->send(response_str);
        std::cout<<"message sended success"<<std::endl;
    }
    else std::cout<<"serialize response_str error"<<std::endl;
    conn->shutdown();
}