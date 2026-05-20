/* -------------------------------------------------------------------------
//  文件名    : GameData.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : SaveApple 数据模型——分数、生命、水果列表、设置
// -------------------------------------------------------------------------*/

#pragma once

#include <exam/IObserver.h>

#include "Fruit.h"

#include <QVector>
#include <QtGlobal>

class GameData {
public:
    // 构造模型：初始化所有游戏状态为默认值。
    GameData();

    // 当前分数。
    int score() const;
    // 当前生命数。
    int lives() const;
    // 是否游戏结束（生命耗尽等）。
    bool isGameOver() const;
    // 是否刚刚“过关”（用于一次性提示）。
    bool isPassed() const;
    // 当前下落速度（由设置面板直接控制）。
    int fallSpeed() const;
    // 同屏最大苹果数量（生成上限）。
    int maxAppleCount() const;
    // 过关目标数量（达到后过关）。
    int targetCount() const;

    // 只读水果列表（用于渲染）。
    const QVector<Fruit>& fruits() const;
    // 可编辑水果列表（控制器更新位置/删除时使用）。
    QVector<Fruit>& editableFruits();

    // 重置一局（清空水果、分数、生命、统计等）。
    void reset();
    // 增加分数。
    void addScore(int value);
    // 扣一条命，并在需要时进入 game over。
    void loseLife();
    // 添加一个水果到场景。
    void addFruit(const Fruit& fruit);
    // 删除指定下标的水果。
    void removeFruitAt(int index);
    // 记录一次命中输入（成功次数+1，并检查是否过关）。
    void recordHit();
    // 检查是否达到过关条件（达标后置 passed）。
    void checkPassProgress();
    // 设置下落速度（内部会做范围限制）。
    void setFallSpeed(int speed);
    // 设置同屏最大苹果数量（内部会做范围限制）。
    void setMaxAppleCount(int count);
    // 设置过关目标数量（内部会做范围限制）。
    void setTargetCount(int count);

    // 注册观察者（View），用于模型变化后通知刷新。
    void attach(IObserver* observer);
    // 取消注册观察者。
    void detach(IObserver* observer);
    // 通知所有观察者模型已变化。
    void notifyObservers();

private:
    int m_score;
    int m_lives;
    bool m_gameOver;
    bool m_passed;
    int m_successCount;
    int m_fallSpeed;
    int m_maxAppleCount;
    int m_targetCount;
    QVector<Fruit> m_fruits;
    QVector<IObserver*> m_observers;
};
