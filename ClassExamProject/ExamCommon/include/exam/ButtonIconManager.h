/* -------------------------------------------------------------------------
//  文件名    : ButtonIconManager.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : 三态按钮图标管理器——精灵图注册、eventFilter 切换、工厂创建
// -------------------------------------------------------------------------*/

#pragma once

#include "exam/IconUtils.h"

#include <QEvent>
#include <QHash>
#include <QIcon>
#include <QObject>
#include <QPushButton>

#include <functional>

class ButtonIconManager : public QObject {
public:
    explicit ButtonIconManager(QObject* parent = nullptr) : QObject(parent) {}

    void registerButton(QPushButton* btn, const QString& spritePath, const QSize& size) {
        if (!btn) return;
        IconState s;
        s.normal  = iconFromThird(spritePath, 0, size);
        s.hover   = iconFromThird(spritePath, 1, size);
        s.pressed = iconFromThird(spritePath, 2, size);
        m_states.insert(btn, s);
        btn->setIcon(s.normal);
        btn->setIconSize(size);
        btn->setFlat(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton{border:none;background:transparent;}"
            "QPushButton:pressed{padding-top:1px;}");
    }

    bool isManaged(QPushButton* btn) const { return m_states.contains(btn); }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        auto* btn = qobject_cast<QPushButton*>(watched);
        if (!btn) return QObject::eventFilter(watched, event);
        auto it = m_states.find(btn);
        if (it == m_states.end()) return QObject::eventFilter(watched, event);

        switch (event->type()) {
        case QEvent::Enter:
            if (!btn->isDown()) btn->setIcon(it->hover);
            if (m_audioCb) m_audioCb(true);
            break;
        case QEvent::Leave:
            if (!btn->isDown()) btn->setIcon(it->normal);
            break;
        case QEvent::MouseButtonPress:
            btn->setIcon(it->pressed);
            break;
        case QEvent::MouseButtonRelease:
            btn->setIcon(btn->underMouse() ? it->hover : it->normal);
            if (btn->underMouse() && m_audioCb) m_audioCb(false);
            break;
        default:
            break;
        }
        return false;
    }

private:
    struct IconState {
        QIcon normal;
        QIcon hover;
        QIcon pressed;
    };
    QHash<QPushButton*, IconState> m_states;
    std::function<void(bool hover)> m_audioCb;

public:
    /// 设置按钮交互音效回调：hover=true 时触发悬停音效，hover=false 时触发点击音效。
    void setAudioCallback(std::function<void(bool hover)> cb) { m_audioCb = std::move(cb); }

    /// 工厂方法：创建按钮并绑定三态图标，一步到位。
    static QPushButton* create(QWidget* parent, const QString& spritePath, const QSize& size,
                               ButtonIconManager* mgr) {
        auto* btn = new QPushButton(parent);
        btn->installEventFilter(mgr);
        mgr->registerButton(btn, spritePath, size.isValid() ? size : QSize(64, 64));
        if (size.isValid()) btn->setFixedSize(size);
        else { btn->setMinimumSize(1, 1); btn->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX); }
        return btn;
    }
};
