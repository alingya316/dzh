/// @file EnemyCaptionLetterFactory.cpp
/// CaptionLetterComposite 与 EnemyCaptionLetterFactory 的实现（Widget / GraphicsItem 双路径）。

#include "View/EnemyCaptionLetterFactory.h"
#include "View/SpriteCrop.h"

#include <QFont>
#include <QFontInfo>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>

namespace {

QFont makeCaptionLetterFont(const CaptionLetterSpec& spec) {
    QFont f(QStringLiteral("Times New Roman"));
    f.setBold(true);
    f.setPointSize(spec.letterPointSize);
    return f;
}

} // namespace

void CaptionLetterComposite::layoutIntoCaptionArea(QLabel* captionLabel,
                                                   QLabel* letterLabel,
                                                   const QPixmap& captionBack,
                                                   QChar letter,
                                                   const CaptionLetterSpec& spec) {
    if (!captionLabel || !letterLabel) {
        return;
    }
    if (!captionBack.isNull()) {
        captionLabel->setPixmap(
            captionBack.scaled(spec.captionW, spec.captionH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    captionLabel->resize(spec.captionW, spec.captionH);

    letterLabel->setFont(makeCaptionLetterFont(spec));
    letterLabel->setStyleSheet(QStringLiteral("color:white;background:transparent;"));
    letterLabel->setText(QString(letter));
    letterLabel->adjustSize();

    const QPoint c = captionLabel->pos();
    letterLabel->move(c.x() + (spec.captionW - letterLabel->width()) / 2,
                       c.y() + (spec.captionH - letterLabel->height()) / 2);
}

void CaptionLetterComposite::addGraphicsComposite(QGraphicsItemGroup* group,
                                                  const QPointF& captionTopLeft,
                                                  const QPixmap& captionBack,
                                                  QChar letter,
                                                  const CaptionLetterSpec& spec) {
    if (!group) {
        return;
    }
    if (!captionBack.isNull()) {
        const QPixmap cap =
            captionBack.scaled(spec.captionW, spec.captionH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        auto* capItem = new QGraphicsPixmapItem(cap);
        capItem->setPos(captionTopLeft);
        group->addToGroup(capItem);
    }
    auto* txt = new QGraphicsTextItem(QString(letter));
    txt->setFont(makeCaptionLetterFont(spec));
    txt->setDefaultTextColor(Qt::white);
    const QRectF br = txt->boundingRect();
    txt->setPos(captionTopLeft.x() + (spec.captionW - br.width()) / 2.0,
                captionTopLeft.y() + (spec.captionH - br.height()) / 2.0);
    group->addToGroup(txt);
}

ClusterLayoutProfile AircraftClusterStrategy::defaultProfile() const {
    ClusterLayoutProfile t;
    // 字幕条 + 字母贴在敌机精灵底部（相对载体左上角），不再叠在头顶。
    t.captionFromCarrierY = qMax(0, t.carrierH - t.composite.captionH);
    return t;
}

ClusterLayoutProfile MeteorClusterStrategy::defaultProfile() const {
    ClusterLayoutProfile t;
    t.carrierW = t.carrierH = m_carrierSize;
    t.composite.captionW = 96;
    t.composite.captionH = 48;
    t.composite.letterPointSize = AircraftClusterStrategy().defaultProfile().composite.letterPointSize;
    t.captionFromCarrierY = m_carrierSize / 2 - 24;
    return t;
}

ClusterLayoutProfile EnemyCaptionLetterFactory::inferFromWidgets(const QLabel* carrier,
                                                                 const QLabel* caption,
                                                                 const QLabel* letter) {
    if (!carrier || !caption || !letter) {
        return AircraftClusterStrategy().defaultProfile();
    }
    ClusterLayoutProfile t;
    t.carrierW = qMax(1, carrier->width());
    t.carrierH = qMax(1, carrier->height());
    const QPoint e = carrier->pos();
    t.composite.captionW = qMax(1, caption->width());
    t.composite.captionH = qMax(1, caption->height());
    t.captionFromCarrierY = caption->y() - e.y();
    {
        const QFontInfo fi(letter->font());
        t.composite.letterPointSize =
            qMax(6, fi.pointSize() > 0 ? fi.pointSize() : qMax(6, fi.pixelSize() * 3 / 4));
    }
    return t;
}

void EnemyCaptionLetterFactory::layoutWidgetCluster(const QPixmap& carrierSheet,
                                                    int animFrame,
                                                    const QPixmap& captionBack,
                                                    QChar letter,
                                                    const ClusterLayoutProfile& profile,
                                                    QLabel* carrierLabel,
                                                    QLabel* captionLabel,
                                                    QLabel* letterLabel) {
    if (!carrierLabel || !captionLabel || !letterLabel) {
        return;
    }
    const QPixmap frame = cropSpriteFrame34(carrierSheet, animFrame);
    if (!frame.isNull()) {
        carrierLabel->setPixmap(
            frame.scaled(profile.carrierW, profile.carrierH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    carrierLabel->resize(profile.carrierW, profile.carrierH);

    const QPoint base = carrierLabel->pos();
    const int capX = base.x() + (profile.carrierW - profile.composite.captionW) / 2;
    const int capY = base.y() + profile.captionFromCarrierY;
    captionLabel->move(capX, capY);

    CaptionLetterComposite::layoutIntoCaptionArea(captionLabel, letterLabel, captionBack, letter, profile.composite);
}

void EnemyCaptionLetterFactory::addGraphicsCluster(QGraphicsItemGroup* group,
                                                   const QPointF& carrierTopLeft,
                                                   const QPixmap& carrierSheet,
                                                   int animFrame,
                                                   const QPixmap& captionBack,
                                                   QChar letter,
                                                   const ClusterLayoutProfile& profile) {
    if (!group) {
        return;
    }
    const QPixmap frame = cropSpriteFrame34(carrierSheet, animFrame);
    if (!frame.isNull()) {
        auto* ci = new QGraphicsPixmapItem(
            frame.scaled(profile.carrierW, profile.carrierH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        ci->setPos(carrierTopLeft);
        group->addToGroup(ci);
    }
    const QPointF captionTopLeft(carrierTopLeft.x() + (profile.carrierW - profile.composite.captionW) / 2.0,
                                 carrierTopLeft.y() + profile.captionFromCarrierY);
    CaptionLetterComposite::addGraphicsComposite(group, captionTopLeft, captionBack, letter, profile.composite);
}
