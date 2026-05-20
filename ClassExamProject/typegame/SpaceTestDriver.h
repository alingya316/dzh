/* -------------------------------------------------------------------------
//  文件名    : SpaceTestDriver.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-03
//  功能描述  : SpaceBattle 无头测试驱动器——驱动游戏循环、模拟按键、输出 JSON
// -------------------------------------------------------------------------*/

#pragma once

#include "Model/SpaceModel.h"

#include <QJsonObject>
#include <QString>

struct CliTestConfig {
    QString letters;
    SpaceSettings settings;
    int testGameDuration = 30;
    int correctRounds = 1;
    int errorRounds = 1;
    int allwrongRounds = 1;
    int errorFrequency = 5;
};

class SpaceTestDriver {
public:
    static CliTestConfig loadConfig(const QString& path);
    static QJsonObject run(const CliTestConfig& cfg);
    static bool saveResult(const QString& path, const QJsonObject& result);
};
