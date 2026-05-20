/* -------------------------------------------------------------------------
//  文件名    : GameWindow.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : 游戏窗口公共配置——模态、退出→回调、X→回调
// -------------------------------------------------------------------------*/

#pragma once

#include "exam/CloseFilter.h"

#include <QWidget>
#include <functional>

/// disconnects exitClicked → oldSlot, reconnects exitClicked and X button → cleanup
template <typename View>
inline void setupGameWindow(View* view,
                            void (View::*exitSignal)(),        // &View::exitClicked
                            const std::function<void()>& cleanup) {
    QObject::disconnect(view, exitSignal, nullptr, nullptr);
    QObject::connect(view, exitSignal, [cleanup] { cleanup(); });
    view->installEventFilter(new CloseFilter(cleanup, view));
    view->setWindowModality(Qt::ApplicationModal);
}
