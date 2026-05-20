/* -------------------------------------------------------------------------
//  文件名    : SpaceController.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : SpaceBattle 控制器——定时器驱动的游戏循环、敌人生成、碰撞检测
// -------------------------------------------------------------------------*/

#pragma once

/// @file SpaceController.h
/// 太空大战 · 控制器（Controller）：定时器驱动物理与碰撞、生成敌人与奖励词、响应 View 信号。

#include "Model/SpaceModel.h"

#include <QObject>
#include <QTimer>

class SpaceView;
class WordProvider;

/// 连接 SpaceModel 与 SpaceView：驱动游戏循环，并把 UI 事件转为模型变更。
class SpaceController : public QObject {
    Q_OBJECT
public:
    explicit SpaceController(SpaceModel* model, SpaceView* view, QObject* parent = nullptr);

public slots:
    /// 主循环（约 60Hz）：积分 dt，更新飞船/敌弹/奖励并刷新界面。
    void tick();
    /// 按间隔生成新敌人（受 maxEnemies、随机陨石/飞机比例约束）。
    void spawnEnemy();
    /// 定时拉升难度：提高上限移速并缩短生成间隔。
    void upgradeDifficulty();
    /// 奖励模式开启时周期性向 WordProvider 索取新单词。
    void spawnRewardWord();

    void onStart();           ///< 开始游戏：reset、开 BGM、启动升级/奖励定时器
    void onPause();          ///< 空格暂停 / 恢复（含 BGM pause）
    void onExit();            ///< 退出应用程序
    void onReturnToMenu();    ///< 回主菜单：停表、reset、状态 Initial
    void onDismissGameOver(); ///< Game Over 全屏确认后回主菜单
    void onSettingsApplied(const SpaceSettings& s); ///< 应用设置并重建相关定时器间隔
    /// 按下 A–Z：对「最靠下」且字母匹配的敌人发射追踪弹。
    void onLetterPressed(QChar c);
    /// 奖励模式下追加输入字符，校验前缀或完成整词回血。
    void onRewardTyped(QChar c);
    /// WordProvider 回调：在屏幕左侧激活奖励词。
    void onWordReady(const QString& word, bool fromApi);

private:
    /// 清空敌人子弹分数时间等，并写回开局前的 SpaceSettings（m_baseSettings）。
    void resetGame();
    void updateShip(double dt);
    void updateEnemies(double dt);
    void updateBullets(double dt);
    void updateReward(double dt);

    int nextEnemyId();
    /// 从当前存活敌人未占用的字母中随机选一个；若 A–Z 均已占用则返回空 QChar。
    QChar nextUniqueLetter() const;

    SpaceModel* m_model = nullptr;
    SpaceView* m_view = nullptr;
    WordProvider* m_wordProvider = nullptr;

    QTimer m_tickTimer;
    QTimer m_spawnTimer;
    QTimer m_upgradeTimer;
    QTimer m_rewardTimer;

    qint64 m_lastMs = 0;
    SpaceSettings m_baseSettings; ///< 点击「开始」前快照，用于 resetGame 恢复玩家选项
    int m_nextEnemyId = 1;
};
