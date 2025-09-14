#include "mprpcconfig.h"
#include <iostream>
#include <string>

void MprpcConfig::LoadConfigfile(const char* config_file){//加载获取存储整个配置文件
    FILE *pf=fopen(config_file,"r");//只读方式打开文件
    if(nullptr==pf){
         std::cout<<"config_file is not exsit"<<std::endl;
         exit(EXIT_FAILURE);
    }
    while(!feof(pf)){//当文件未读取完（判断指针是否到文件末尾）
        char buf[512]={0};
        fgets(buf,512,pf);//一行一行读文件
        //去掉字符串前多余空格
        std::string read_buf(buf);//初始化缓冲区
        Trim(read_buf);
        //判断#注释
        if(read_buf[0]=='#'||read_buf.empty())continue;
        //解析配置项
        int idx=read_buf.find('=');
        if(idx==-1) continue;
        std::string key;
        std::string value;
        key=read_buf.substr(0,idx);
        Trim(key);
        int endidx=read_buf.find("\n",idx);
        value=read_buf.substr(idx+1,endidx-idx-1);
        Trim(value);
        m_configMap.insert({key,value});
        
    }
}

std::string MprpcConfig::Load(const std::string &key){//获取配置文件中的某个值
    auto it=m_configMap.find(key);
    if(it==m_configMap.end()) return "";
    return it->second;
}

void MprpcConfig::Trim(std::string &src_buf){
    int idx=src_buf.find_first_not_of(' ');
    if(idx!=-1)
        src_buf=src_buf.substr(idx, src_buf.size()-idx);
    idx=src_buf.find_last_not_of(' ');
    if(idx!=-1)
        src_buf=src_buf.substr(0, idx+1);
}