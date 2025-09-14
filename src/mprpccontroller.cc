#include "mprpccontroller.h"

MprpcController::MprpcController(){
    m_failed=false;//开始没有出错
    m_errText="";
}

void MprpcController::Reset(){//重置成初始状态
    m_failed=false;
    m_errText="";
}

bool MprpcController::Failed() const{//判断当前成功与否，返回当前m_failed的值
    return m_failed;
}

std::string MprpcController::ErrorText() const{//当前错误码
    return m_errText;
}

void MprpcController::SetFailed(const std::string& reason){//设置为出错状态
    m_failed=true;
    m_errText=reason;//错误信息
}

//目前尚未实现的功能
void MprpcController::StartCancel(){}
bool MprpcController::IsCanceled() const {return false;}
void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback){}