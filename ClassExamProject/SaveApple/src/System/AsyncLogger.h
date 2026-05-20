/* -------------------------------------------------------------------------
//  文件名    : AsyncLogger.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : 异步日志记录器——后台线程写入日志文件
// -------------------------------------------------------------------------*/

#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>

class AsyncLogger {
public:
    static AsyncLogger& instance();

    void log(const std::string& level, const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);

    ~AsyncLogger();

private:
    AsyncLogger();
    AsyncLogger(const AsyncLogger&) = delete;
    AsyncLogger& operator=(const AsyncLogger&) = delete;

    void workerLoop();

    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::deque<std::string> m_pendingLines;
    std::thread m_worker;
    bool m_stop;
    std::string m_logFilePath;
};
