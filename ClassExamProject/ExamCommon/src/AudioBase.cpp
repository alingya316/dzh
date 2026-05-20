/* -------------------------------------------------------------------------
//  文件名    : AudioBase.cpp
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : AudioBase 实现——BGM 创建与生命周期
// -------------------------------------------------------------------------*/

#include "exam/AudioBase.h"

#include <QApplication>
#include <QMediaPlayer>
#include <QSoundEffect>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioOutput>
#endif

void AudioBase::startBgm()   { if (m_bgm) { m_bgm->setMuted(false); m_bgm->play(); } }
void AudioBase::stopBgm()    { if (m_bgm) { m_bgm->setMuted(true); m_bgm->stop(); } }
void AudioBase::pauseBgm()   { if (m_bgm) m_bgm->pause(); }
void AudioBase::resumeBgm()  { if (m_bgm) m_bgm->play(); }

void AudioBase::ensureBgm(const char* qrc, qreal volume) {
    if (!QApplication::instance() || m_bgm) return;
    if (!m_owner)
        m_owner = new QObject(QApplication::instance());
    m_bgm = new QMediaPlayer(m_owner);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto* out = new QAudioOutput(m_owner);
    out->setVolume(static_cast<float>(volume));
    m_bgm->setAudioOutput(out);
    m_bgm->setSource(QUrl(qrc));
    m_bgm->setLoops(QMediaPlayer::Infinite);
#else
    m_bgm->setVolume(static_cast<int>(volume * 100));
    m_bgm->setMedia(QUrl(qrc));
#endif
}

void AudioBase::ensureSfx(QPointer<QSoundEffect>& sfx, const char* qrc, float vol) {
    if (!QApplication::instance() || sfx) return;
    if (!m_owner)
        m_owner = new QObject(QApplication::instance());
    sfx = new QSoundEffect(m_owner);
    sfx->setSource(QUrl(qrc));
    sfx->setVolume(vol);
}
