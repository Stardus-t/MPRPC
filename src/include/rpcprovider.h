#pragma once
#include "google/protobuf/service.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>


//框架提供的专门服务发布rpc服务的网络对象类
class RpcProvider{
public:
    //框架提供给外部使用的用于发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service * service);//这里接收所有服务的基类
    //启动RPC服务
    void Run();
private:
    muduo::net::EventLoop m_eventloop;
    //服务类型信息
    struct ServiceInfo{
        google::protobuf::Service *m_service;//保存服务对象
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor*> m_MethodMap;//保存服务方法
    };
    //存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string,ServiceInfo> m_serviceMap;
    //连接回调函数
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    //读写回调函数
    void OnMessage(const muduo::net::TcpConnectionPtr&,muduo::net::Buffer*,muduo::Timestamp);
    //Closure回调操作,用于序列化rpc响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&,google::protobuf::Message*);

};