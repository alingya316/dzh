#include "AnalyticsManager.h"

#include "AsyncLogger.h"

#include <QDateTime>

#include <sstream>

AnalyticsManager& AnalyticsManager::instance() {
    static AnalyticsManager manager;
    return manager;
}

AnalyticsManager::AnalyticsManager()
    : m_stop(false),
      m_sessionRunning(false) {
    m_worker = std::thread(&AnalyticsManager::workerLoop, this);
}

AnalyticsManager::~AnalyticsManager() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    m_cv.notify_one();
    if (m_worker.joinable()) {
        m_worker.join();
    }
}

void AnalyticsManager::startSession() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_counter.clear();
    m_queue.clear();
    m_sessionRunning = true;
    AsyncLogger::instance().info("analytics session started");
}

void AnalyticsManager::finishSession(int finalScore, bool passed) {
    recordEvent("final_score", finalScore);
    recordEvent(passed ? "pass_count" : "fail_count", 1);
    flushSnapshot("session_finish");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sessionRunning = false;
}

void AnalyticsManager::recordEvent(const std::string& eventName, int delta) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_sessionRunning) {
            return;
        }
        m_queue.push_back({eventName, delta});
    }
    m_cv.notify_one();
}

void AnalyticsManager::workerLoop() {
    while (true) {
        std::unordered_map<std::string, int> snapshot;
        {
            std::deque<Event> batch;
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this]() { return m_stop || !m_queue.empty(); });
            if (m_stop && m_queue.empty()) {
                break;
            }
            batch.swap(m_queue);
            for (const Event& event : batch) {
                m_counter[event.name] += event.delta;
            }
            snapshot = m_counter;
        }

        std::ostringstream oss;
        oss << "analytics_tick";
        for (const auto& entry : snapshot) {
            oss << " " << entry.first << "=" << entry.second;
        }
        AsyncLogger::instance().info(oss.str());
    }
}

void AnalyticsManager::flushSnapshot(const std::string& reason) {
    std::unordered_map<std::string, int> snapshot;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        snapshot = m_counter;
    }

    std::ostringstream oss;
    oss << "analytics_flush reason=" << reason
        << " at=" << QDateTime::currentDateTime().toString("hh:mm:ss").toStdString();
    for (const auto& entry : snapshot) {
        oss << " " << entry.first << "=" << entry.second;
    }
    AsyncLogger::instance().info(oss.str());
}
