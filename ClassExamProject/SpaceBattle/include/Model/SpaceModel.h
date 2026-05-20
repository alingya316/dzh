/* -------------------------------------------------------------------------
//  文件名    : SpaceModel.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : SpaceBattle 数据模型
// -------------------------------------------------------------------------*/

#pragma once

/// @file SpaceModel.h
/// 太空大战 · 数据模型（Model）：仅存状态与配置，不含游戏规则逻辑；规则在 SpaceController。
///
/// 与 SaveApple 共用 ClassExamProject/ExamCommon：若需 Model→View 被动刷新，可对齐
/// `<exam/IObserver.h>` 与 GameData 的 attach/notify 用法（SpaceBattle 当前由 Controller 主动调 refreshUi）。

#include <QChar>
#include <QPointF>
#include <QString>
#include <QVector>

/// 游戏流程状态：主菜单 / 进行中 / 空格暂停 / 局内 Game Over（仍显示战场）/ 结束（保留枚举供测试）。
enum class SpaceGameState {
    Initial,
    Playing,
    Paused,
    GameOver,
    End,
};

/// 敌人类型：飞机（横向摆动下落）或陨石（竖直下落；不与敌机相撞，可与飞船相撞）。
enum class EnemyType { Aircraft, Meteor };

/// 单个敌人实例：位置、速度、头顶待输入字母、存活与爆炸动画帧等。
struct SpaceEnemy {
    int id = 0;
    EnemyType type = EnemyType::Aircraft;
    QPointF pos;
    QPointF vel;
    QChar letter;
    bool alive = true;
    bool exploding = false;
    int explosionFrame = 0;
    int animFrame = 0;
};

/// 追踪弹：朝锁定敌人飞行，命中后敌方进入爆炸状态。
struct SpaceBullet {
    QPointF pos;
    QPointF vel;
    int targetEnemyId = -1;
    bool alive = true;
};

/// 奖励模式：屏幕掠过的英文单词，玩家顺序键入正确后回复生命。
struct RewardWord {
    QString word;
    QPointF pos;
    qreal speed = 140.0;
    bool active = false;
};

/// 可在设置面板调整的参数（音量由 SpaceAudio 内部默认，不设模型字段）。
struct SpaceSettings {
    int maxEnemies = 5;       ///< 场上敌人数上限 1–10
    int speedLevel = 5;       ///< 下落速度档位 1–10
    int upgradeIntervalSec = 20; ///< 难度升级定时器间隔（秒）
    bool rewardMode = false;  ///< 是否启用英文单词奖励关
};

/// 聚合分数、生命、飞船、敌人列表、子弹与奖励词等，供 View 读取、Controller 修改。
class SpaceModel {
public:
    /// 回到初始统计状态；保留 settings（具体恢复由 Controller 用 m_baseSettings 处理）。
    void reset();

    SpaceGameState state() const { return m_state; }
    void setState(SpaceGameState s) { m_state = s; }

    int score() const { return m_score; }
    void addScore(int delta) { m_score += delta; }

    int life() const { return m_life; }
    void setLife(int life) { m_life = life; }

    double elapsedSec() const { return m_elapsedSec; }
    void addElapsed(double dt) { m_elapsedSec += dt; }

    QPointF shipPos() const { return m_shipPos; }
    void setShipPos(const QPointF& p) { m_shipPos = p; }
    QPointF shipVel() const { return m_shipVel; }
    void setShipVel(const QPointF& v) { m_shipVel = v; }

    /// 飞船精灵序列帧（3×4 图集内前 11 格循环）。
    int shipAnimFrame() const { return m_shipAnimFrame; }
    void setShipAnimFrame(int f) { m_shipAnimFrame = (f % 11 + 11) % 11; }
    void advanceShipAnimFrame() { m_shipAnimFrame = (m_shipAnimFrame + 1) % 11; }

    QVector<SpaceEnemy>& enemies() { return m_enemies; }
    const QVector<SpaceEnemy>& enemies() const { return m_enemies; }

    QVector<SpaceBullet>& bullets() { return m_bullets; }
    const QVector<SpaceBullet>& bullets() const { return m_bullets; }

    RewardWord& rewardWord() { return m_rewardWord; }
    const RewardWord& rewardWord() const { return m_rewardWord; }

    SpaceSettings& settings() { return m_settings; }
    const SpaceSettings& settings() const { return m_settings; }

    /// 奖励模式下玩家已输入的字母缓冲（需与 rewardWord.word 前缀匹配）。
    QString& typedRewardBuffer() { return m_typedReward; }
    const QString& typedRewardBuffer() const { return m_typedReward; }

private:
    SpaceGameState m_state = SpaceGameState::Initial;
    int m_score = 0;
    int m_life = 18;
    double m_elapsedSec = 0.0;

    QPointF m_shipPos{0, 0};
    QPointF m_shipVel{160.0, 0}; ///< 默认横向往返速度（Controller 每帧会校正）
    int m_shipAnimFrame = 0;

    QVector<SpaceEnemy> m_enemies;
    QVector<SpaceBullet> m_bullets;

    RewardWord m_rewardWord;
    QString m_typedReward;
    SpaceSettings m_settings;
};
