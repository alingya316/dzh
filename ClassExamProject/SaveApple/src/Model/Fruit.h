/* -------------------------------------------------------------------------
//  文件名    : Fruit.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : 水果实体——类型、位置、速度、字母、动画帧
// -------------------------------------------------------------------------*/

#pragma once

#include <QPointF>
#include <QChar>
#include <QRectF>
#include <QSize>

enum class FruitType {
    // 普通苹果：玩家正确输入可得分。
    GoodApple,
    // 小苹果：可作为不同分值/体积的普通苹果变体。
    SmallApple,
    // 坏苹果：命中/规则可做惩罚或不计分对象。
    BadApple,
};

class Fruit {
public:
    // 创建水果：指定类型、初始位置（中心点）和下落速度。
    Fruit(FruitType type, QPointF position, qreal speed);

    // 水果类型（普通/小/坏）。
    FruitType type() const;
    // 设置水果类型。
    void setType(FruitType type);

    // 水果中心点位置（用于绘制与碰撞）。
    QPointF position() const;
    // 设置水果中心点位置。
    void setPosition(const QPointF& position);

    // 下落速度（像素/秒或按 deltaTime 的单位定义）。
    qreal speed() const;
    // 设置下落速度。
    void setSpeed(qreal speed);

    // “碎裂动画”剩余帧数（>0 表示还在碎裂表现中）。
    int smashFrames() const;
    // 设置碎裂帧数。
    void setSmashFrames(int frames);
    // 碎裂帧数递减 1（到 0 停止）。
    void tickSmashFrame();
    // 显示在苹果上的字母（用于键盘命中）。
    QChar letter() const;
    // 设置苹果字母。
    void setLetter(QChar letter);

    // 更新水果运动（按 deltaTime 移动位置）。
    void update(float deltaTime);
    // 计算包围盒（用于碰撞判定），spriteSize 为贴图尺寸。
    QRectF bounds(const QSize& spriteSize) const;
    // 该水果的得分值（命中时加多少分）。
    int scoreValue() const;

private:
    FruitType m_type;
    QPointF m_position;
    qreal m_speed;
    int m_smashFrames;
    QChar m_letter;
};
