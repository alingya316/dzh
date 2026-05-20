#include "AsyncLogger.h"

#include <QCoreApplication>
#include <QDateTime>

#include <filesystem>
#include <fstream>

AsyncLogger& AsyncLogger::instance() {
    static AsyncLogger logger;
    return logger;
}

AsyncLogger::AsyncLogger()
    : m_stop(false) {
    const QString baseDir = QCoreApplication::applicationDirPath() + "/logs";
    std::filesystem::create_directories(baseDir.toStdString());
    m_logFilePath = (baseDir + "/game.log").toStdString();
    m_worker = std::thread(&AsyncLogger::workerLoop, this);
}

AsyncLogger::~AsyncLogger() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    m_cv.notify_one();
    if (m_worker.joinable()) {
        m_worker.join();
    }
}

void AsyncLogger::log(const std::string& level, const std::string& message) {
    const QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    const std::string line = "[" + ts.toStdString() + "] [" + level + "] " + message;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pendingLines.push_back(line);
    }
    m_cv.notify_one();
}

void AsyncLogger::info(const std::string& message) {
    log("INFO", message);
}

void AsyncLogger::warn(const std::string& message) {
    log("WARN", message);
}

void AsyncLogger::error(const std::string& message) {
    log("ERROR", message);
}

void AsyncLogger::workerLoop() {
    std::ofstream out(m_logFilePath, std::ios::app);
    if (!out.is_open()) {
        return;
    }

    while (true) {
        std::deque<std::string> localBatch;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this]() { return m_stop || !m_pendingLines.empty(); });
            if (m_stop && m_pendingLines.empty()) {
                break;
            }
            localBatch.swap(m_pendingLines);
        }

        for (const std::string& line : localBatch) {
            out << line << '\n';
        }
        out.flush();
    }
}
