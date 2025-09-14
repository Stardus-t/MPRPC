#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>
#include <zookeeper/zookeeper.h>

//全局的watcher观察器
void global_watcher(zhandle_t *zh,int type,int state,const char *path,void *watcherCtx){
    if(type==ZOO_SESSION_EVENT){
        if(state==ZOO_CONNECTED_STATE){//zkServer和zkCLient连接成功
            sem_t *sem=(sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

//zoo_aexist的回调，处理节点是否存在
void exists_completion(int rc,const struct Stat *stat,const void *data){
    struct ExistsData* exist_data=(struct ExistsData*)data;
    exist_data->rc=rc;
    sem_post(exist_data->sem);//释放信号量，唤醒等待的主线程
}

//zoo_acreate的回调，处理节点创建的结果
void create_completion(int rc,const char *path,const void *data){
    struct CreateData* create_data = (struct CreateData*)data;
    create_data->rc = rc;
    sem_post(create_data->sem);
}   

//zoo_aget的回调，处理获取节点数据的结果
void get_completion(int rc,const char *value,int value_len,const struct Stat *stat,const void *data){
    struct GetData* get_data=(struct GetData*)data;
    get_data->rc=rc;
    if(rc==ZOK&&value_len!=0) get_data->result=std::string(value,value_len);
    sem_post(get_data->sem);
}

ZkClient::ZkClient():m_zhandle(nullptr){

}

ZkClient::~ZkClient(){
    if(m_zhandle!=nullptr) zookeeper_close(m_zhandle);//关闭句柄，释放资源
}

//连接ZkServer
void ZkClient::Start(){
    std::string host=MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port=MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr=host+":"+port;
    //连接zookeeperServer
    m_zhandle=zookeeper_init(connstr.c_str(),global_watcher,3000,nullptr,nullptr,0);//3000:会话的超时时间
    //检测是否创建句柄资源成功
    if(nullptr==m_zhandle){
        std::cout<<"zookeeper_init error!"<<std::endl;
        exit(EXIT_FAILURE);
    }
    sem_t sem;//信号量
    sem_init(&sem,0,0);
    zoo_set_context(m_zhandle,&sem);
    sem_wait(&sem);//等待信号量。watcher里连接server成功后，信号量+1，继续往下走
    std::cout<<"zookeeper_init success!"<<std::endl;
}

void ZkClient::Create(const char *path,const char *data,int datalen,int state){
    // char path_buffer[128];
    // int bufferlen=sizeof(path_buffer);
    sem_t sem;
    sem_init(&sem, 0, 0);
    struct ExistsData exists_data;
    exists_data.sem=&sem;
    exists_data.rc=0;

    //先判断path表示的znode节点是否存在，如果存在，就不重复创建
    int flag=zoo_aexists(m_zhandle,path,0,exists_completion,&exists_data);
    if(flag!=ZOK){
        std::cout<<"zoo_aexists error for path: "<<path<<std::endl;
        sem_destroy(&sem);
        return ;
    }
    sem_wait(&sem);
    if (exists_data.rc == ZNONODE) {
        struct CreateData create_data;
        create_data.sem = &sem;
        create_data.rc = 0;
        
        flag = zoo_acreate(m_zhandle, path, data, datalen, 
                          &ZOO_OPEN_ACL_UNSAFE, state, 
                          create_completion, &create_data);
        if (flag != ZOK) {
            std::cout << "zoo_acreate error for path: " << path << std::endl;
            sem_destroy(&sem);
            return;
        }
        
        sem_wait(&sem); // 等待创建完成
        
        if (create_data.rc == ZOK) 
            std::cout << "znode create success...path: " << path << std::endl;
        else 
            std::cout << "znode create error... path: " << path 
                      << ", error code: " << create_data.rc << std::endl;
    } else if (exists_data.rc == ZOK)
        std::cout << "znode already exists...path: " << path << std::endl;
    else 
        std::cout << "zoo_aexists error for path: " << path 
                  << ", error code: " << exists_data.rc << std::endl;
    
    
    sem_destroy(&sem);

}

std::string ZkClient::GetData(const char *path){
    sem_t sem;
    sem_init(&sem, 0, 0);
    struct GetData get_data;
    get_data.sem = &sem;
    get_data.rc = 0;
    get_data.result = "";
    int flag = zoo_aget(m_zhandle, path, 0, get_completion, &get_data);
    if (flag != ZOK) {
        std::cout << "zoo_aget error... path: " << path << std::endl;
        sem_destroy(&sem);
        return "";
    }
    
    sem_wait(&sem); // 等待获取数据完成
    sem_destroy(&sem); // 释放信号量资源
    
    if (get_data.rc == ZOK) {
        return get_data.result;
    } else {
        std::cout << "get znode data error... path: " << path 
                  << ", error code: " << get_data.rc << std::endl;
        return "";
    }
}