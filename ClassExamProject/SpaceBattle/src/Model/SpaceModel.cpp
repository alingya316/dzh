/// @file SpaceModel.cpp
/// SpaceModel::reset 实现：清空局内数据，settings 由 Controller 在 resetGame 中写回。

#include "Model/SpaceModel.h"

void SpaceModel::reset() {
    m_state = SpaceGameState::Initial;
    m_score = 0;
    m_life = 18;
    m_elapsedSec = 0.0;
    m_shipPos = QPointF(0, 0);
    m_shipVel = QPointF(160.0, 0);
    m_shipAnimFrame = 0;
    m_enemies.clear();
    m_bullets.clear();
    m_rewardWord = RewardWord{};
    m_typedReward.clear();
    // Settings are preserved across resets; caller restores via m_baseSettings
}

