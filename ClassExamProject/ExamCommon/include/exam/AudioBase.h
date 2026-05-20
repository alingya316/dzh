/* -------------------------------------------------------------------------
//  文件名    : AudioBase.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : 游戏音频基类——惰性创建 QSoundEffect / QMediaPlayer，子类实现 ensure()
// -------------------------------------------------------------------------*/

#pragma once

#include <QObject>
#include <QPointer>
#include <QUrl>

class QMediaPlayer;
class QSoundEffect;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class QAudioOutput;
#endif

class AudioBase : public QObject {
public:
    virtual void ensure() = 0;

    void startBgm();
    void stopBgm();
    void pauseBgm();
    void resumeBgm();

protected:
    explicit AudioBase(QObject* parent = nullptr) : QObject(parent) {}

    void ensureBgm(const char* qrc, qreal volume = 0.45);
    void ensureSfx(QPointer<QSoundEffect>& sfx, const char* qrc, float vol);

    QPointer<QObject> m_owner;
    QPointer<QMediaPlayer> m_bgm;
};
