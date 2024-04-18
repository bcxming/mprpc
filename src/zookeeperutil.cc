#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>

//zkserver给zkclient的通知
//session timeout   30s     zkclient 网络I/O线程  1/3 * timeout 时间发送ping消息
void global_watcher(zhandle_t *zh, int type,
                   int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)  // 回调的消息类型是和会话相关的消息类型
	{
		if (state == ZOO_CONNECTED_STATE)  // zkclient和zkserver连接成功
		{
			sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
		}
	}
}

//注册watcher
void dataChangeWatcher(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx) {
	if (type == ZOO_CHANGED_EVENT) {
		ZkClient* zkCli = static_cast<ZkClient*>(watcherCtx);
		std::unordered_map<std::string, std::vector<std::string>>* host_map = (std::unordered_map<std::string, std::vector<std::string>>*)zoo_get_context(zh);
		std::cout << "节点发生变化了 " << path << std::endl;
		vector<string> children = zkCli->GetAllData(path);
		if (children.size() == 0) {
			(*host_map)[path] = {}; // 清空缓存的节点数据
			return;
		}
		vector<string> nodeData;
		for (int i = 0; i < children.size(); i++) {
			string childPath = std::string(path) + "/" + children[i];
			string data = zkCli->GetData(childPath.c_str());
			nodeData.push_back(data);
		}
		(*host_map)[path] = nodeData; // 更新缓存的节点数据
	}
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle); // 关闭句柄，释放资源
    }
}

// 连接zkserver
void ZkClient::Start()
{
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;
    
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle) 
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);

    sem_wait(&sem);
	zoo_set_context(m_zhandle, &host_map);
    std::cout << "zookeeper_init success!" << std::endl;
}

bool ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
	// 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
	flag = zoo_exists(m_zhandle, path, 0, nullptr);
	if (ZNONODE == flag) // 表示path的znode节点不存在
	{
		// 创建指定path的znode节点了
		flag = zoo_create(m_zhandle, path, data, datalen,
			&ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
		if (flag == ZOK)
		{
			std::cout << "znode create success... path:" << path << std::endl;
			return true;
		}
		else
		{
			std::cout << "flag:" << flag << std::endl;
			std::cout << "znode create error... path:" << path << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	return false;
}

// 根据指定的path，获取znode节点的值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
	int bufferlen = sizeof(buffer);
	int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
	if (flag != ZOK)
	{
		std::cout << "get znode error... path:" << path << std::endl;
		return "";
	}
	else
	{
		return buffer;
	}
}

//获取所有结点
std::vector<std::string> ZkClient::GetAllData(const char *path){
	std::vector<std::string> providers;
	struct String_vector children;
	int ret = zoo_get_children(m_zhandle, path, 0, &children);
	    if (ret == ZOK) {
        for (int i = 0; i < children.count; ++i) {
            providers.push_back(children.data[i]);
        }
    } else {
        std::cerr << "Failed to get children of " << path << std::endl;
    }
	return providers;
}

//给channel使用的
std::string ZkClient::GetHostData(std::string path){
	if(host_map.find(path)==host_map.end()){
		std::vector<std::string> children = this->GetAllData(path.c_str());
		if (children.size()==0)
		{
			return "";
		}
		std::vector<std::string> nodeData;
		for (int i=0;i<children.size();i++) {
			std::string childPath = path + "/" + children[i];
			std::string data = this->GetData(childPath.c_str());
			nodeData.push_back(data);
		}
		host_map[path] = nodeData;
		int rc = zoo_wexists(m_zhandle, path.c_str(), dataChangeWatcher , this, nullptr);
		if (rc != 0){
			std::cout<<"注册watch失败"<<std::endl;
		}
	}
	currentIndex++;
	return host_map[path][currentIndex% host_map[path].size()];
}
