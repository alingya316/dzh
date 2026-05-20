/* -------------------------------------------------------------------------
//  文件名    : IconUtils.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : 横向三等分精灵图裁切为 QIcon
// -------------------------------------------------------------------------*/

#pragma once

#include <QIcon>
#include <QPixmap>

/// 从横向三等分精灵图（normal | hover | pressed）中裁切第 thirdIndex 片并转为 QIcon。
/// thirdIndex: 0=normal, 1=hover, 2=pressed。
/// targetSize 有效时，裁切后缩放到该尺寸。
inline QIcon iconFromThird(const QString& path, int thirdIndex, const QSize& targetSize = QSize()) {
    QPixmap src(path);
    if (src.isNull()) return QIcon();
    const int w = src.width() / 3;
    if (w <= 0) {
        return targetSize.isValid()
            ? QIcon(src.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation))
            : QIcon(src);
    }
    const int idx = qBound(0, thirdIndex, 2);
    QPixmap cropped = src.copy(idx * w, 0, w, src.height());
    if (targetSize.isValid())
        cropped = cropped.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    return QIcon(cropped);
}
