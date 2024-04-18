#pragma once
#include <iostream>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <functional>
using namespace std;
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sys/time.h> 
#define LOG(level, message) logger.log(LogLevel::level, message, __FILE__, __LINE__)
#define LOG_INFO(message) logger.log(INF0, message, __FILE__, __LINE__)

// 日志级别枚举
enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

// 日志条目结构体
struct LogEntry {
    LogLevel level;
    std::string message;
    std::string time;
    std::string filename;
    int line;
};
static std::string formatTime()
{
    //用于格式化记录日志的时间，并将格式化后的时间信息添加到日志流对象中。
    struct timeval tv;
    time_t time;
    char str_t[26] = {0};
    gettimeofday (&tv, NULL);
    time = tv.tv_sec;
    struct tm* p_time = localtime(&time);   
    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S", p_time);
    return std::string(str_t);
}

// 日志系统类
class Logger {
public:
    // 构造函数
    Logger() : currentBuffer_(0), writing_(true), thread_(std::bind(&Logger::flushLog, this)) {
        condition_.notify_one();
    }
    ~Logger(){
        writing_ = false;
        condition_.notify_one();
        thread_.join();
    }

    // 记录日志函数
    void log(LogLevel level, const std::string& message, const char* filename, int line) {
        std::lock_guard<std::mutex> lock(mutex_);
        buffers_[currentBuffer_].emplace_back(LogEntry{level, message, formatTime(),filename,line});
    }

    // 刷新日志函数
    void flushLog() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            // 等待条件满足（缓冲区非空或者写入结束）
            condition_.wait(lock, [this]() { return !buffers_[currentBuffer_].empty() || !writing_; });

            // 切换缓冲区
            auto& buffer = buffers_[currentBuffer_];
            currentBuffer_ = (currentBuffer_ + 1) % 2;

            // 释放锁
            lock.unlock();

            // 将缓冲区的日志写入到文件
            writeToFile(buffer);

            // 清空缓冲区
            buffer.clear();

            // 如果写入结束，则退出循环
            if (!writing_ && buffer.empty()) {
                break;
            }
        }
    }

private:
    // 将缓冲区的日志写入到文件
    void writeToFile(const std::vector<LogEntry>& buffer) {
        std::ofstream file("log.log", std::ios::app); // 打开日志文件，追加模式
        if (file.is_open()) {
            for (const auto& entry : buffer) {
                file << entry.time<<" "<< entry.filename << ":" << entry.line;
                switch (entry.level) {
                    case LogLevel::INFO:
                        file << "[INFO] ";
                        break;
                    case LogLevel::WARNING:
                        file << "[WARNING] ";
                        break;
                    case LogLevel::ERROR:
                        file << "[ERROR] ";
                        break;
                }
                file <<std::endl;
                file << entry.message << std::endl;
            }
            file.close();
        }
    }

private:
    std::mutex mutex_; // 互斥量
    std::condition_variable condition_; // 条件变量
    std::vector<LogEntry> buffers_[2]; // 双缓冲区
    int currentBuffer_; // 当前活跃的缓冲区索引
    bool writing_; // 是否正在写入日志
    std::thread thread_; // 日志写入线程
};