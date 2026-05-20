/* -------------------------------------------------------------------------
//  文件名    : AppleAudio.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : SaveApple 音频（惰性单例）——继承 AudioBase 复用 BGM 逻辑
// -------------------------------------------------------------------------*/

#pragma once

#include "exam/AudioBase.h"

enum class AppleAudioEvent { ButtonHover, ButtonClick, AppleIn };

class AppleAudio : public AudioBase {
public:
    static AppleAudio& instance();
    void play(AppleAudioEvent e);

private:
    AppleAudio() = default;
    void ensure();

    QPointer<QSoundEffect> m_sfxHover, m_sfxClick, m_sfxAppleIn;
};
