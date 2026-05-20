/* -------------------------------------------------------------------------
//  文件名    : AnalyticsManager.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : 埋点管理器单例——事件计数与会话统计
// -------------------------------------------------------------------------*/

#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

class AnalyticsManager {
public:
    static AnalyticsManager& instance();

    void startSession();
    void finishSession(int finalScore, bool passed);
    void recordEvent(const std::string& eventName, int delta = 1);

    ~AnalyticsManager();

private:
    struct Event {
        std::string name;
        int delta;
    };

    AnalyticsManager();
    AnalyticsManager(const AnalyticsManager&) = delete;
    AnalyticsManager& operator=(const AnalyticsManager&) = delete;

    void workerLoop();
    void flushSnapshot(const std::string& reason);

    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::deque<Event> m_queue;
    std::unordered_map<std::string, int> m_counter;
    std::thread m_worker;
    bool m_stop;
    bool m_sessionRunning;
};
