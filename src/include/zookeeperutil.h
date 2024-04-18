#pragma once
#include <unordered_map>
#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>
#include <vector>
#include <functional>

// 封装的zk客户端类
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // zkclient启动连接zkserver
    void Start();
    // 在zkserver上根据指定的path创建znode节点
    bool Create(const char *path, const char *data, int datalen, int state=0);
    // 根据参数指定的znode节点路径，或者znode节点的值
    std::string GetData(const char *path);
    zhandle_t* getzhandle_t(){return m_zhandle;}
    std::vector<std::string> GetAllData(const char *path);
    std::string GetHostData(std::string path);
    
private:
    // zk的客户端句柄
    zhandle_t *m_zhandle;
    int currentIndex = 0;
    std::unordered_map<std::string,std::vector<std::string>>host_map;
};