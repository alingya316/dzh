/* -------------------------------------------------------------------------
//  文件名    : cli_mode.cpp
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : CLI 测试模式实现——Space/Apple 自动化对战
// -------------------------------------------------------------------------*/

#include "cli_mode.h"
#include "AppleTestDriver.h"
#include "SpaceTestDriver.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include <cstdio>

int runSpaceCliTest(const QString& inPath, const QString& outPath) {
    CliTestConfig cfg = SpaceTestDriver::loadConfig(inPath);
    QJsonObject result = SpaceTestDriver::run(cfg);
    if (!SpaceTestDriver::saveResult(outPath, result)) {
        std::fprintf(stderr, "typegame: failed to save result\n");
        return 6;
    }
    std::printf("typegame: test complete — %s\n",
                qPrintable(result["result"].toString()));
    return result["allPassed"].toBool() ? 0 : 1;
}

int runAppleCliTest(const QString& inPath, const QString& outPath) {
    QFile inf(inPath);
    if (!inf.open(QIODevice::ReadOnly)) return 4;
    QJsonObject cfg = QJsonDocument::fromJson(inf.readAll()).object();
    QString letters = cfg.value("letters").toString();
    int duration = cfg.value("testGameDuration").toInt(30);

    QJsonObject result = AppleTestDriver::run(letters, duration);
    if (!AppleTestDriver::saveResult(outPath, result)) return 6;
    std::printf("typegame: test complete — %s\n",
                qPrintable(result["result"].toString()));
    return result["allPassed"].toBool() ? 0 : 1;
}
