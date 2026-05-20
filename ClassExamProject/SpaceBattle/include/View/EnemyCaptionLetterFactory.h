/* -------------------------------------------------------------------------
//  文件名    : EnemyCaptionLetterFactory.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : 敌机+字幕+字母组合渲染工厂
// -------------------------------------------------------------------------*/

#pragma once

/// @file EnemyCaptionLetterFactory.h
/// 敌人头顶「字幕条 + 目标字母」的组合布局（Composite）与敌机/陨石两套默认参数（Strategy）。
/// 同时提供 QLabel 编辑预览与 QGraphicsScene 运行时绘制两套 API。

#include <QChar>
#include <QGraphicsItemGroup>
#include <QLabel>
#include <QPixmap>
#include <QPoint>
#include <QPointF>

/// 字幕框内字母的字体与框尺寸。
struct CaptionLetterSpec {
    int captionW = 80;
    int captionH = 50;
    int letterPointSize = 14;
};

/// 载体（敌机或陨石精灵）尺寸，以及字幕条相对载体顶边的垂直偏移（水平始终居中）。
struct ClusterLayoutProfile {
    CaptionLetterSpec composite;
    int carrierW = 128;
    int carrierH = 128;
    int captionFromCarrierY = 3;
};

/// 组合体：将 SPACE_CAPTION_BACK 与字母 QLabel/QGraphicsTextItem 叠放在框内居中。
class CaptionLetterComposite {
public:
    static void layoutIntoCaptionArea(QLabel* captionLabel,
        QLabel* letterLabel,
        const QPixmap& captionBack,
        QChar letter,
        const CaptionLetterSpec& spec);

    static void addGraphicsComposite(QGraphicsItemGroup* group,
        const QPointF& captionTopLeft,
        const QPixmap& captionBack,
        QChar letter,
        const CaptionLetterSpec& spec);
};

/// Strategy 接口：返回该类载体推荐的 ClusterLayoutProfile。
class IClusterCarrierStrategy {
public:
    virtual ~IClusterCarrierStrategy() = default;
    virtual ClusterLayoutProfile defaultProfile() const = 0;
};

/// 敌机默认布局（较小字幕条）。
class AircraftClusterStrategy final : public IClusterCarrierStrategy {
public:
    ClusterLayoutProfile defaultProfile() const override;
};

/// 陨石载体为正方形时的默认布局（字幕更宽，垂直位置偏下）。
class MeteorClusterStrategy final : public IClusterCarrierStrategy {
    int m_carrierSize = 128;

public:
    explicit MeteorClusterStrategy(int carrierSize = 128) : m_carrierSize(carrierSize) {}
    ClusterLayoutProfile defaultProfile() const override;
};

/// 工厂：从 QWidget 拖拽控件反推 Profile；或为 QLabel/QGraphicsItemGroup 批量摆放精灵+字幕+字母。
class EnemyCaptionLetterFactory {
public:
    static ClusterLayoutProfile inferFromWidgets(const QLabel* carrier,
        const QLabel* caption,
        const QLabel* letter);

    static void layoutWidgetCluster(const QPixmap& carrierSheet,
        int animFrame,
        const QPixmap& captionBack,
        QChar letter,
        const ClusterLayoutProfile& profile,
        QLabel* carrierLabel,
        QLabel* captionLabel,
        QLabel* letterLabel);

    static void addGraphicsCluster(QGraphicsItemGroup* group,
        const QPointF& carrierTopLeft,
        const QPixmap& carrierSheet,
        int animFrame,
        const QPixmap& captionBack,
        QChar letter,
        const ClusterLayoutProfile& profile);
};

using EnemyCaptionLetterTemplate = ClusterLayoutProfile;
