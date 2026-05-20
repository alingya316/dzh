#pragma once

/// @file SpriteCrop.h
/// 从飞船/敌机共用的 3×4 精灵表中截取当前动画格（前 11 格循环，第 12 格不使用）。

#include <QPixmap>
#include <QtGlobal>

inline QPixmap cropSpriteFrame34(const QPixmap& sheet, int animFrame) {
    constexpr int kCols = 3;
    constexpr int kRows = 4;
    constexpr int kLoopFrames = 11;
    if (sheet.isNull()) {
        return {};
    }
    const int fw = sheet.width() / kCols;
    const int fh = sheet.height() / kRows;
    if (fw <= 0 || fh <= 0) {
        return {};
    }
    const int idx = animFrame % kLoopFrames;
    const int col = idx % kCols;
    const int row = idx / kCols;
    return sheet.copy(col * fw, row * fh, fw, fh);
}
