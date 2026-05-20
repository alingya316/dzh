/* -------------------------------------------------------------------------
//  文件名    : AppleTestDriver.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : SaveApple 无头测试驱动器——CLI 模式自动对战
// -------------------------------------------------------------------------*/

#pragma once

#include <QJsonObject>
#include <QString>

class AppleTestDriver {
public:
    static QJsonObject run(const QString& letters, int gameDurationSec);
    static bool saveResult(const QString& path, const QJsonObject& result);
};
