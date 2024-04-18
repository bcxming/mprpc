#pragma once

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <unordered_map>
#include "zookeeperutil.h"
using namespace std;
class MprpcChannel : public google::protobuf::RpcChannel
{
public:
    // 所有通过stub代理对象调用的rpc方法，都走到这里了，统一做rpc方法调用的数据数据序列化和网络发送 
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, 
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response,
                          google::protobuf:: Closure* done);
    MprpcChannel(){
    zkCli.Start();
    }
private:
    ZkClient zkCli;

};


class RoundRobinLoadBalancer {
private:
    std::vector<std::string> providers_; // 存储服务提供者的地址列表
    std::atomic<size_t> current_index_;   // 当前选择的服务提供者的索引
    // 构造函数，接受一个服务提供者地址列表
    RoundRobinLoadBalancer()
        : providers_({}), current_index_(0) {}
    ~RoundRobinLoadBalancer() {
        // 析构函数
    }
public:

    static RoundRobinLoadBalancer& getInstance() {
        // 使用静态局部变量实现懒汉式单例模式
        // 在第一次调用该方法时，静态局部变量会被初始化
        static RoundRobinLoadBalancer instance;
        return instance;
    }
    // 禁止拷贝构造和赋值运算符
    RoundRobinLoadBalancer(const RoundRobinLoadBalancer&) = delete;
    RoundRobinLoadBalancer& operator=(const RoundRobinLoadBalancer&) = delete;
    // 选择下一个服务提供者的地址
    std::string SelectProvider() {
        // 使用原子操作获取当前索引，并增加索引以准备下一次选择
        size_t index = current_index_++;

        // 使用取余运算来确定下一个服务提供者的索引，实现循环选择
        return providers_[index % providers_.size()];
    }
    void setProvider(const std::vector<std::string>& providers){
        providers_ = providers;
    }
};