#pragma once 
#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string.h>
#include <iostream>

struct ExistsData{
    sem_t *sem;//信号量，用于同步
    int rc;//操作结果，如ZNONODE,ZOK
};

struct CreateData{
    sem_t *sem;//信号量，用于同步
    int rc;//操作结果，如ZNONODE,ZOK
};

struct GetData{
    sem_t *sem;//信号量，用于同步
    int rc;//操作结果，如ZNONODE,ZOK
    std::string result;//存储获取到的节点数据
};


//封装的zk客户端类
class ZkClient{
public:
    ZkClient();
    ~ZkClient();
    //启动连接zkServer
    void Start();
    //在zkServer上根据指定的path创建zNode节点
    void Create(const char *path,const char *data,int datalen,int state=0);//state: 是否为永久性节点
    //根据参数指定zNode节点路径，或该节点的值
    std::string GetData(const char *path);
private:
    //zk的客户端句柄
    zhandle_t *m_zhandle;
};