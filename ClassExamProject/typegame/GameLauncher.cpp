#include "GameLauncher.h"

#include <QApplication>
#include <QMouseEvent>
#include <QScreen>
#include <QPainter>
#include <QRadialGradient>
#include <QTimer>

// Design reference: 740 x 500
static constexpr qreal kRefW = 740.0;
static constexpr qreal kRefH = 500.0;
static constexpr qreal kRefCardW = 310.0;
static constexpr qreal kRefCardH = 340.0;
static constexpr qreal kRefCardGap = 36.0;
static constexpr qreal kRefIconMax = 195.0;
static constexpr qreal kRefTitleAreaH = 95.0;
static constexpr qreal kRefTitleFontPt = 26.0;
static constexpr qreal kRefInfoFontPt = 8.0;
static constexpr qreal kRefCardNameFontPt = 11.0;
static constexpr qreal kRefCardSubFontPt = 10.0;
static constexpr qreal kRefHintFontPt = 9.0;

GameLauncher::GameLauncher(QWidget* parent) : QWidget(parent) {
    const QScreen* screen = QApplication::primaryScreen();
    const int h = screen ? screen->availableGeometry().height() * 3 / 4 : 500;
    setFixedSize(h * 4 / 3, h);
    setWindowTitle("TypeGame");
    setMouseTracking(true);

    m_cards[0].title = "拯救苹果";
    m_cards[0].subtitle = "Save Apple";
    m_cards[0].icon = QPixmap(":/launcher/APPLE.PNG");

    m_cards[1].title = "太空大战";
    m_cards[1].subtitle = "Space Battle";
    m_cards[1].icon = QPixmap(":/launcher/SPACE.png");

    auto* animTimer = new QTimer(this);
    animTimer->setInterval(16);
    connect(animTimer, &QTimer::timeout, this, [this]() {
        bool needsUpdate = false;
        for (int i = 0; i < 2; ++i) {
            qreal target = (i == m_hoveredIndex) ? 1.0 : 0.0;
            qreal diff = target - m_cards[i].hover;
            if (qAbs(diff) > 0.01) {
                m_cards[i].hover += diff * 0.18;
                needsUpdate = true;
            } else {
                m_cards[i].hover = target;
            }
        }
        if (needsUpdate) update();
    });
    animTimer->start();

    layoutCards();
}

static inline qreal sx(int w) { return static_cast<qreal>(w) / kRefW; }
static inline qreal sy(int h) { return static_cast<qreal>(h) / kRefH; }

void GameLauncher::layoutCards() {
    const qreal sw = width() / kRefW;
    const qreal sh = height() / kRefH;
    const int cardW = static_cast<int>(kRefCardW * sw);
    const int cardH = static_cast<int>(kRefCardH * sh);
    const int cardGap = static_cast<int>(kRefCardGap * sw);
    const int titleAreaH = static_cast<int>(kRefTitleAreaH * sh);
    const int totalW = cardW * 2 + cardGap;
    const int startX = (width() - totalW) / 2;
    const int startY = titleAreaH + (height() - titleAreaH - cardH) / 2;

    for (int i = 0; i < 2; ++i) {
        m_cards[i].rect = QRectF(startX + i * (cardW + cardGap), startY, cardW, cardH);
    }
}

void GameLauncher::resizeEvent(QResizeEvent*) { layoutCards(); }

void GameLauncher::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const qreal sw = width() / kRefW;
    const qreal sh = height() / kRefH;

    // Background
    p.fillRect(rect(), QColor(18, 22, 40));

    const int cardW = static_cast<int>(kRefCardW * sw);
    const int cardH = static_cast<int>(kRefCardH * sh);
    const int titleAreaH = static_cast<int>(kRefTitleAreaH * sh);
    const int cardGap = static_cast<int>(kRefCardGap * sw);
    const int totalW = cardW * 2 + cardGap;
    const int leftCardCx = (width() - totalW) / 2 + cardW / 2;
    const int rightCardCx = leftCardCx + cardW + cardGap;
    const int cardCy = titleAreaH + (height() - titleAreaH - cardH) / 2 + cardH / 2;

    // Soft glows behind cards
    p.setPen(Qt::NoPen);
    QRadialGradient gLeft(leftCardCx, cardCy, static_cast<int>(250 * sw));
    gLeft.setColorAt(0, QColor(55, 75, 130, 30));
    gLeft.setColorAt(1, QColor(55, 75, 130, 0));
    p.setBrush(gLeft);
    p.drawEllipse(QPointF(leftCardCx, cardCy), 250 * sw, 250 * sh);

    QRadialGradient gRight(rightCardCx, cardCy, static_cast<int>(250 * sw));
    gRight.setColorAt(0, QColor(55, 75, 130, 30));
    gRight.setColorAt(1, QColor(55, 75, 130, 0));
    p.setBrush(gRight);
    p.drawEllipse(QPointF(rightCardCx, cardCy), 250 * sw, 250 * sh);

    // Top accent line
    p.fillRect(0, 0, width(), 3, QColor(100, 140, 220));

    // Title area gradient
    QLinearGradient titleGrad(0, 0, 0, titleAreaH);
    titleGrad.setColorAt(0, QColor(24, 30, 50));
    titleGrad.setColorAt(1, QColor(18, 22, 40));
    p.fillRect(0, 0, width(), titleAreaH, titleGrad);

    // Title - use generous rect to avoid clipping
    QFont titleFont("Microsoft YaHei", static_cast<int>(kRefTitleFontPt * sh), QFont::Bold);
    p.setFont(titleFont);
    p.setPen(QColor(230, 235, 245));
    p.drawText(QRectF(0, 8 * sh, width(), titleAreaH - 16 * sh), Qt::AlignCenter, "游戏启动器");

    // Separator line
    p.setPen(QPen(QColor(70, 85, 115, 80), 1));
    p.drawLine(QPointF(width() * 0.15, titleAreaH - 1), QPointF(width() * 0.85, titleAreaH - 1));

    // Bottom info — placed just above window bottom, clear of cards
    QFont infoFont("Microsoft YaHei", static_cast<int>(kRefInfoFontPt * sh));
    p.setFont(infoFont);
    p.setPen(QColor(75, 85, 105));
    const int bottomH = static_cast<int>(24 * sh);
    p.drawText(QRectF(0, height() - bottomH, width(), bottomH), Qt::AlignCenter, "点击卡片启动游戏");

    // Cards
    const int iconMax = static_cast<int>(kRefIconMax * qMin(sw, sh));
    for (int i = 0; i < 2; ++i) {
        const Card& card = m_cards[i];
        const QRectF r = card.rect;
        const qreal h = card.hover;

        p.save();
        if (h > 0.002) {
            qreal s = 1.0 + h * 0.04;
            p.translate(r.center());
            p.scale(s, s);
            p.translate(-r.center());
        }

        // Shadow
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 25 + static_cast<int>(h * 35)));
        p.drawRoundedRect(r.adjusted(0, 4 * sh, 0, 4 * sh), 16 * sw, 16 * sh);

        // Card body
        p.setBrush(QColor(
            static_cast<int>(28 + 14 * h), static_cast<int>(34 + 16 * h),
            static_cast<int>(54 + 26 * h), static_cast<int>(230 + 10 * h)));
        p.setPen(QPen(QColor(
            static_cast<int>(55 + 65 * h), static_cast<int>(68 + 92 * h),
            static_cast<int>(100 + 130 * h), static_cast<int>(120 + 80 * h)),
            1.5));
        p.drawRoundedRect(r, 16 * sw, 16 * sh);

        // Icon tray
        const qreal trayH = r.width() - 24 * sw;
        const QRectF trayRect(r.left() + 10 * sw, r.top() + 14 * sh,
                              r.width() - 20 * sw, trayH);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, 6 + static_cast<int>(h * 10)));
        p.drawRoundedRect(trayRect, 10 * sw, 10 * sh);

        // Icon
        if (!card.icon.isNull()) {
            const qreal s = 1.0 + h * 0.03;
            const qreal iw = iconMax * s;
            const qreal ih = iconMax * s;
            const QPixmap scaled = card.icon.scaled(
                static_cast<int>(iw), static_cast<int>(ih),
                Qt::KeepAspectRatio, Qt::SmoothTransformation);
            p.drawPixmap(
                static_cast<int>(r.center().x() - scaled.width() / 2.0),
                static_cast<int>(trayRect.center().y() - scaled.height() / 2.0),
                scaled);
        }

        // Card title — close to tray, generous height
        const qreal textBaseY = trayRect.bottom() + 2 * sh;
        const qreal textRemaining = r.bottom() - textBaseY;
        const qreal titleH = textRemaining * 0.55;
        QFont cardNameFont("Microsoft YaHei",
                           static_cast<int>((kRefCardNameFontPt + h * 1.5) * sh), QFont::Bold);
        p.setFont(cardNameFont);
        p.setPen(QColor(225, 230, 240));
        p.drawText(QRectF(r.left(), textBaseY, r.width(), titleH),
                   Qt::AlignCenter, card.title);

        // Card subtitle
        QFont cardSubFont("Segoe UI", static_cast<int>(kRefCardSubFontPt * sh));
        p.setFont(cardSubFont);
        p.setPen(QColor(135, 150, 180));
        p.drawText(QRectF(r.left(), textBaseY + titleH, r.width(), textRemaining - titleH),
                   Qt::AlignCenter, card.subtitle);

        // Hover hint — overlaid on upper part of icon area
        if (h > 0.3) {
            QFont hintFont("Microsoft YaHei", static_cast<int>(kRefHintFontPt * sh));
            p.setFont(hintFont);
            p.setPen(QColor(120, 180, 240, static_cast<int>(h * 150)));
            const qreal hintY = trayRect.top() + trayRect.height() * 0.05;
            const qreal hintH = trayRect.height() * 0.2;
            p.drawText(QRectF(r.left(), hintY, r.width(), hintH),
                       Qt::AlignCenter, QStringLiteral("▶  启  动"));
        } else {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(90, 100, 125, 70));
            const qreal dotY = r.bottom() - textRemaining * 0.2;
            for (int dot = 0; dot < 3; ++dot) {
                p.drawEllipse(QPointF(r.center().x() + (dot - 1) * 10 * sw, dotY),
                              2.5 * sw, 2.5 * sh);
            }
        }

        // Glow ring on strong hover
        if (h > 0.6) {
            QColor ring(110, 160, 235, static_cast<int>((h - 0.6) * 90));
            p.setPen(QPen(ring, 2.5));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(r.adjusted(2 * sw, 2 * sh, -2 * sw, -2 * sh),
                              14 * sw, 14 * sh);
        }

        p.restore();
    }
}

void GameLauncher::mouseMoveEvent(QMouseEvent* e) {
    int prev = m_hoveredIndex;
    m_hoveredIndex = -1;
    for (int i = 0; i < 2; ++i) {
        if (m_cards[i].rect.contains(e->pos())) {
            m_hoveredIndex = i;
            break;
        }
    }
    if (m_hoveredIndex != prev) {
        setCursor(m_hoveredIndex >= 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
}

void GameLauncher::leaveEvent(QEvent*) {
    m_hoveredIndex = -1;
    setCursor(Qt::ArrowCursor);
    update();
}

void GameLauncher::mousePressEvent(QMouseEvent* e) {
    for (int i = 0; i < 2; ++i) {
        if (m_cards[i].rect.contains(e->pos())) {
            launchGame(i);
            return;
        }
    }
}

void GameLauncher::launchGame(int index) {
    emit gameSelected(index);
}
