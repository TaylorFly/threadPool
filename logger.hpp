#pragma once

#include "lockqueue.hpp"
#include <string>

#define __ENABLE_LOG__

#define LOG_INFO(logmsgformat, ...) \
    do {\
        Logger &logger = Logger::getInstance();\
        logger.setLogLevel(INFO);\
        char s[1024] = {0}; \
        sprintf(s, logmsgformat, ##__VA_ARGS__); \
        logger.log(std::string(s) + "\n");\
    } while (0);

#define LOG_ERROR(logmsgformat, ...) \
    do {\
        Logger &logger = Logger::getInstance();\
        logger.setLogLevel(ERROR)\
        char s[1024] = {0}; \
        sprintf(s, logmsgformat, ##__VA_ARGS__); \
        logger.log(std::string(s) + "\n");\
    } while (0);


enum LogLevel {
    INFO, ERROR
};

/**
* 日志系统
*/
class Logger {
public:
    //设置日志级别
    void setLogLevel(LogLevel level);
    //写日志
    void log(const std::string &msg); 
    static Logger& getInstance();
private:
    LogLevel logLevel_;  //记录日志级别
    LockQueue<std::string> que_; //日志缓冲队列

    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger &&) = delete;
};