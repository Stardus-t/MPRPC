#pragma once 
#include <google/protobuf/service.h>
#include <google/protobuf/message.h>

class MprpcChannel: public google::protobuf::RpcChannel{
public:
    //所有通过stub调用框架的客户端都要用该类的callback(数据序列化和网络发送)
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done);
};