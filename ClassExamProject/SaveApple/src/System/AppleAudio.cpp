/* -------------------------------------------------------------------------
//  文件名    : AppleAudio.cpp
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : AppleAudio 实现——惰性创建 SFX，BGM 委托 AudioBase
// -------------------------------------------------------------------------*/

#include "AppleAudio.h"

#include <QApplication>
#include <QSoundEffect>

AppleAudio& AppleAudio::instance() {
    static AppleAudio audio;
    return audio;
}

void AppleAudio::ensure() {
    if (!QApplication::instance()) return;

    ensureBgm("qrc:/sounds/sounds/APPLE_BG.wav", 0.45);
    ensureSfx(m_sfxHover,   "qrc:/sounds/sounds/ANIBTN_ENTER.wav", 0.8f);
    ensureSfx(m_sfxClick,   "qrc:/sounds/sounds/BTN_CLICK.wav",    0.9f);
    ensureSfx(m_sfxAppleIn, "qrc:/sounds/sounds/APPLE_IN.wav",     0.9f);
}

void AppleAudio::play(AppleAudioEvent e) {
    ensure();
    switch (e) {
    case AppleAudioEvent::ButtonHover: if (m_sfxHover)   m_sfxHover->play();   break;
    case AppleAudioEvent::ButtonClick: if (m_sfxClick)   m_sfxClick->play();   break;
    case AppleAudioEvent::AppleIn:     if (m_sfxAppleIn) m_sfxAppleIn->play(); break;
    }
}
