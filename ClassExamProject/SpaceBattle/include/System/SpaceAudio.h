/* -------------------------------------------------------------------------
//  文件名    : SpaceAudio.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : SpaceBattle 音频（惰性单例）——继承 AudioBase 复用 BGM 逻辑
// -------------------------------------------------------------------------*/

#pragma once

#include "exam/AudioBase.h"

enum class SpaceAudioEvent { Shoot, Blast, PlaneOut, WordOut, Upgrade, ButtonHover, ButtonClick };

class SpaceAudio : public AudioBase {
public:
    static SpaceAudio& instance();
    void play(SpaceAudioEvent e);
    void setBgmVolume(qreal vol);
    void setSfxVolume(qreal vol);

private:
    SpaceAudio() = default;
    void ensure();

    QPointer<QSoundEffect> m_shoot, m_blast, m_planeOut, m_wordOut, m_upgrade, m_btnHover, m_btnClick;
    qreal m_bgmVolume = 0.9, m_sfxVolume = 0.9;
    qreal m_sfxBaseShoot = 0.9, m_sfxBaseBlast = 0.9, m_sfxBasePlaneOut = 0.9, m_sfxBaseWordOut = 0.8, m_sfxBaseUpgrade = 0.8;
};
