/* -------------------------------------------------------------------------
//  文件名    : CloseFilter.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : 拦截窗口关闭事件（X 按钮），转为自定义回调
// -------------------------------------------------------------------------*/

#pragma once

#include <QEvent>
#include <QObject>

#include <functional>

class CloseFilter : public QObject {
    std::function<void()> m_cleanup;
public:
    explicit CloseFilter(std::function<void()> cleanup, QObject* parent = nullptr)
        : QObject(parent), m_cleanup(std::move(cleanup)) {}
protected:
    bool eventFilter(QObject*, QEvent* e) override {
        if (e->type() == QEvent::Close) {
            e->ignore();
            m_cleanup();
            return true;
        }
        return false;
    }
};
