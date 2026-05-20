#include "GameConfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

namespace {
QString detectProjectImageRoot() {
    // 基于当前源码文件路径推导资源目录，不依赖运行时工作目录。
    const QFileInfo thisFile(QString::fromUtf8(__FILE__));  // .../src/Config/GameConfig.cpp
    QDir dir = thisFile.dir();                              // .../src/Config
    dir.cdUp();                                             // .../src
    dir.cdUp();                                             // .../SaveApple
    const QString root = dir.absoluteFilePath("resources/images");
    return QDir::fromNativeSeparators(root);                // .../SaveApple/resources/images
}
}  // namespace

GameConfig& GameConfig::instance() {
    static GameConfig config;
    return config;
}

int GameConfig::initialLives() const {
    return 3;
}

int GameConfig::spawnIntervalMs() const {
    return 900;
}

int GameConfig::updateIntervalMs() const {
    return 16;
}

QString GameConfig::resourceRoot() const {
    static const QString root = detectProjectImageRoot();
    return root;
}

QString GameConfig::imagePath(const QString& filename) const {
    const QString root = resourceRoot();
    if (root.isEmpty()) {
        return {};
    }
    const QString flatFile = root + "/" + filename;
    if (QFileInfo::exists(flatFile)) {
        return flatFile;
    }
    return {};
}

QString GameConfig::commonImagePath(const QString& filename) const {
    return imagePath(filename);
}
