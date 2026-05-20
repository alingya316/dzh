/* -------------------------------------------------------------------------
//  文件名    : SpaceAudio.cpp
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : SpaceAudio 实现——惰性创建 SFX，BGM 委托 AudioBase
// -------------------------------------------------------------------------*/

#include "System/SpaceAudio.h"

#include <QApplication>
#include <QSoundEffect>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioOutput>
#endif
#include <QMediaPlayer>

SpaceAudio& SpaceAudio::instance() {
    static SpaceAudio audio;
    return audio;
}

void SpaceAudio::ensure() {
    if (!QApplication::instance()) return;

    // BGM via base class
    ensureBgm("qrc:/space/sounds/sounds/SPACE_BG.wav", static_cast<qreal>(m_bgmVolume));

    // SFX with volume scaling
    ensureSfx(m_shoot,    "qrc:/space/sounds/sounds/SPACE_SHOOT.wav",    static_cast<float>(m_sfxBaseShoot * m_sfxVolume));
    ensureSfx(m_blast,    "qrc:/space/sounds/sounds/SPACE_BLAST.wav",    static_cast<float>(m_sfxBaseBlast * m_sfxVolume));
    ensureSfx(m_planeOut, "qrc:/space/sounds/sounds/SPACE_PLANEOUT.wav", static_cast<float>(m_sfxBasePlaneOut * m_sfxVolume));
    ensureSfx(m_wordOut,  "qrc:/space/sounds/sounds/SPACE_WORDOUT.wav",  static_cast<float>(m_sfxBaseWordOut * m_sfxVolume));
    ensureSfx(m_upgrade,  "qrc:/space/sounds/sounds/UPGRADE.wav",        static_cast<float>(m_sfxBaseUpgrade * m_sfxVolume));
    ensureSfx(m_btnHover, "qrc:/sounds/sounds/ANIBTN_ENTER.wav",         0.8f);
    ensureSfx(m_btnClick, "qrc:/sounds/sounds/BTN_CLICK.wav",            0.9f);
}

void SpaceAudio::play(SpaceAudioEvent e) {
    ensure();
    switch (e) {
    case SpaceAudioEvent::Shoot:       if (m_shoot)    m_shoot->play();    break;
    case SpaceAudioEvent::Blast:       if (m_blast)    m_blast->play();    break;
    case SpaceAudioEvent::PlaneOut:    if (m_planeOut) m_planeOut->play(); break;
    case SpaceAudioEvent::WordOut:     if (m_wordOut)  m_wordOut->play();  break;
    case SpaceAudioEvent::Upgrade:     if (m_upgrade)  m_upgrade->play();  break;
    case SpaceAudioEvent::ButtonHover: if (m_btnHover) m_btnHover->play(); break;
    case SpaceAudioEvent::ButtonClick: if (m_btnClick) m_btnClick->play(); break;
    }
}

void SpaceAudio::setBgmVolume(qreal vol) {
    m_bgmVolume = qBound(0.0, vol, 1.0);
    if (m_bgm) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if (auto* out = m_bgm->audioOutput()) out->setVolume(static_cast<float>(m_bgmVolume));
#else
        m_bgm->setVolume(static_cast<int>(m_bgmVolume * 100));
#endif
    }
}

void SpaceAudio::setSfxVolume(qreal vol) {
    m_sfxVolume = qBound(0.0, vol, 1.0);
    auto set = [&](QPointer<QSoundEffect>& sfx, qreal base) {
        if (sfx) sfx->setVolume(static_cast<float>(base * m_sfxVolume));
    };
    set(m_shoot, m_sfxBaseShoot); set(m_blast, m_sfxBaseBlast);
    set(m_planeOut, m_sfxBasePlaneOut); set(m_wordOut, m_sfxBaseWordOut);
    set(m_upgrade, m_sfxBaseUpgrade);
}
