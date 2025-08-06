#include "logger.hpp"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <thread>
#include <time.h>

Logger& Logger::getInstance() {
    static Logger logger;
    return logger;
}

//启动写日志线程
Logger::Logger() {
    std::thread writeLogTask([&]() {
        while (true) {
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);
            char file_name[128];
            sprintf(file_name, "%4d-%02d-%02d-log.txt", 
                                nowtm->tm_year + 1900,
                                nowtm->tm_mon + 1,
                                nowtm->tm_mday);
                        
            FILE *fp = fopen(file_name, "a+");
            if (fp == nullptr) {
                std::cout << "logger file: " << file_name << " open error\n";
                exit(EXIT_FAILURE);
            }
            std::string msg = que_.pop();
            char time_buf[128] = {0};
            sprintf(time_buf, "[%02d:%02d:%02d] => [%s] ", nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec, logLevel_ == ERROR ? "ERROR" : "INFO");
            msg.insert(0, time_buf);
            fputs(msg.c_str(), fp);
            fputs(msg.c_str(), stdout);
            fclose(fp);
        }
    });
    writeLogTask.detach();
}

void Logger::setLogLevel(LogLevel level) {
    logLevel_ = level;
}

void Logger::log(const std::string &msg) {
    que_.push(msg);
}

