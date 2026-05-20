/// @file SpaceView.cpp
/// 菜单与战斗 UI：资源加载、高分文件 space.hi、QGraphics 场景分层（静态 HUD + 动态实体）。

#include "View/SpaceView.h"
#include "View/SpriteCrop.h"

#include "exam/ButtonIconManager.h"
#include "exam/SettingsHelper.h"
#include "System/SpaceAudio.h"

#include <QApplication>
#include <QDebug>
#include <QEvent>

#include <algorithm>
#include <QFrame>
#include <QGraphicsEllipseItem>
#include <QGraphicsItemGroup>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QRandomGenerator>
#include <QRect>
#include <QVector>
#include <QtGlobal>
#include <QFile>
#include <QSlider>
#include <QTextStream>

namespace {

QPixmap loadImg(const QString& name) {
    return QPixmap(QString(":/space/images/images/") + name);
}

// 与 layoutAll 中 overlay 面板比例一致（参考窗 1704×1278 → 内层 1278×958）
constexpr int kLayoutRefW = 1704;
constexpr int kLayoutRefH = 1278;

QSize spaceSettingsFooterBtnSize(int viewW, int viewH) {
    if (viewW <= 0 || viewH <= 0) {
        viewW = kLayoutRefW;
        viewH = kLayoutRefH;
    }
    return QSize(viewW * 226 / kLayoutRefW, viewH * 98 / kLayoutRefH);
}

QSize spaceHiscoreBackBtnSize(int viewW, int viewH) {
    if (viewW <= 0 || viewH <= 0) {
        viewW = kLayoutRefW;
        viewH = kLayoutRefH;
    }
    return QSize(viewW * 332 / kLayoutRefW, viewH * 68 / kLayoutRefH);
}

/// SPACE_STARS：5 种星。支持 (5×1)、(1×5) 像素条；或宽/高为 5 的整数倍时沿该方向五等分整格。
constexpr int kStarTypeCount = 5;

bool buildScaledStarVariants(const QPixmap& strip, int drawSide, QVector<QPixmap>* out) {
    out->clear();
    if (strip.isNull() || drawSide < 1) return false;
    const int W = strip.width(), H = strip.height();
    QVector<QRect> cells;
    cells.reserve(kStarTypeCount);
    if (W == kStarTypeCount && H >= 1) {
        for (int i = 0; i < kStarTypeCount; ++i) cells.append(QRect(i, 0, 1, H));
    } else if (H == kStarTypeCount && W >= 1) {
        for (int i = 0; i < kStarTypeCount; ++i) cells.append(QRect(0, i, W, 1));
    } else if (W >= kStarTypeCount && (W % kStarTypeCount == 0) && H >= 1) {
        const int cw = W / kStarTypeCount;
        for (int i = 0; i < kStarTypeCount; ++i) cells.append(QRect(i * cw, 0, cw, H));
    } else if (H >= kStarTypeCount && (H % kStarTypeCount == 0) && W >= 1) {
        const int ch = H / kStarTypeCount;
        for (int i = 0; i < kStarTypeCount; ++i) cells.append(QRect(0, i * ch, W, ch));
    } else {
        return false;
    }
    for (const QRect& r : cells) {
        const QPixmap cell = strip.copy(r);
        if (cell.isNull()) return false;
        out->append(cell.scaled(drawSide, drawSide, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    }
    return out->size() == kStarTypeCount;
}
} // namespace

// ======================== Constructor ========================
SpaceView::SpaceView(SpaceModel* model, QWidget* parent)
    : QWidget(parent)
    , m_model(model)
    , m_btnIcons(new ButtonIconManager(this))
{
    m_btnIcons->setAudioCallback([](bool hover) {
        SpaceAudio::instance().play(hover ? SpaceAudioEvent::ButtonHover
                                          : SpaceAudioEvent::ButtonClick);
    });
    m_enemyClusterTemplate = AircraftClusterStrategy().defaultProfile();
    m_meteorCaptionFromCarrierY =
        MeteorClusterStrategy(m_enemyClusterTemplate.carrierW).defaultProfile().captionFromCarrierY;
    setFocusPolicy(Qt::StrongFocus);
    loadAssets();
    setupUi();
    setFocus();
}

void SpaceView::loadAssets() {
    m_bg          = loadImg("SPACE_BACKGROUND.png");
    m_menuBg      = loadImg("SPACE_MAINMENU_BG.png");
    m_ship        = loadImg("SPACE_SHIP.png");
    m_enemyA      = loadImg("SPACE_ENEMY_0.png");
    m_meteorFull  = loadImg("SPACE_ENEMY_4.png");
    m_explosionFull=loadImg("SPACE_EXPLOSION_0.png");
    m_captionBack = loadImg("SPACE_CAPTION_BACK.png");
    m_bomb        = loadImg("SPACE_BOMB.png");
    m_labelLife   = loadImg("SPACE_LABEL_LIFE.png");
    m_labelScore  = loadImg("SPACE_LABEL_SCORE.png");
    m_labelTime   = loadImg("SPACE_LABEL_TIME.png");
    m_lifeBar     = loadImg("SPACE_LIFE.png");
    m_lifeOver    = loadImg("SPACE_LIFE_OVER.png");
    m_stars       = loadImg("SPACE_STARS.png");

    // 飞船 / 敌机：整张 3×4 序列图，绘制时按帧裁剪（前 11 格循环）

    // Slice checkbox: 80x20 → 4 parts of 20x20
    QPixmap cb = loadImg("CHECKBOX_BUTTON.png");
    if (!cb.isNull()) {
        int s = cb.width() / 4;
        m_checkUnchecked      = cb.copy(0, 0, s, s);
        m_checkUncheckedHover = cb.copy(s, 0, s, s);
        m_checkChecked        = cb.copy(s * 2, 0, s, s);
        m_checkCheckedHover   = cb.copy(s * 3, 0, s, s);
    }
    loadScores();
}

void SpaceView::loadScores() {
    m_highScores.clear();
    QFile f(QApplication::applicationDirPath() + "/space.hi");
    if (f.open(QIODevice::ReadOnly)) {
        QTextStream in(&f);
        while (!in.atEnd()) {
            bool ok; int s = in.readLine().trimmed().toInt(&ok);
            if (ok) m_highScores.append(s);
        }
        f.close();
    }
    std::sort(m_highScores.begin(), m_highScores.end(), std::greater<int>());
    m_highScores = m_highScores.mid(0, 9);
}

void SpaceView::saveScore(int score) {
    if (score <= 0) return;
    m_highScores.append(score);
    std::sort(m_highScores.begin(), m_highScores.end(), std::greater<int>());
    m_highScores = m_highScores.mid(0, 9);
    QFile f(QApplication::applicationDirPath() + "/space.hi");
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QTextStream out(&f);
        for (int s : m_highScores) out << s << "\n";
        f.close();
    } else {
        qWarning() << "SpaceView: cannot save high scores to" << f.fileName();
    }
}

// ======================== setupUi ========================
void SpaceView::setupUi() {
    // Game view — fills entire SpaceView, underneath all widgets
    m_gameScene = new QGraphicsScene(this);
    m_gameView = new QGraphicsView(m_gameScene, this);
    m_gameView->setFrameShape(QFrame::NoFrame);
    m_gameView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_gameView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_gameView->setFocusPolicy(Qt::NoFocus);
    m_gameView->setStyleSheet("background: transparent;");
    m_gameView->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_staticGroup = new QGraphicsItemGroup();
    m_dynamicGroup = new QGraphicsItemGroup();
    m_gameScene->addItem(m_staticGroup);
    m_gameScene->addItem(m_dynamicGroup);

    // Menu buttons — 2x original (238x46 → 476x92)
    const QSize btnSize(590, 98);
    m_btnStart   = ButtonIconManager::create(this, ":/space/images/images/SPACE_START.png", btnSize, m_btnIcons);
    m_btnHiscore = ButtonIconManager::create(this, ":/space/images/images/SPACE_HISCORE.png", btnSize, m_btnIcons);
    m_btnOption  = ButtonIconManager::create(this, ":/space/images/images/SPACE_OPTION.png", btnSize, m_btnIcons);
    m_btnExit    = ButtonIconManager::create(this, ":/space/images/images/SPACE_EXIT.png", btnSize, m_btnIcons);

    connect(m_btnStart,   &QPushButton::clicked, this, &SpaceView::startClicked);
    connect(m_btnHiscore, &QPushButton::clicked, this, &SpaceView::showHiscorePanel);
    connect(m_btnOption,  &QPushButton::clicked, this, &SpaceView::showSettings);
    connect(m_btnExit,    &QPushButton::clicked, this, &SpaceView::exitClicked);

    // Game Over label
    m_gameOverOverlay = new QWidget(this);
    m_gameOverOverlay->setStyleSheet(QStringLiteral("background-color:rgba(0,0,0,140);"));
    m_gameOverOverlay->hide();
    m_gameOverOverlay->installEventFilter(this);
    m_gameOverLabel = new QLabel(QStringLiteral("GAME OVER"), m_gameOverOverlay);
    QFont gof(QStringLiteral("Times New Roman")); gof.setBold(true); gof.setPointSize(36);
    m_gameOverLabel->setFont(gof);
    m_gameOverLabel->setStyleSheet(QStringLiteral("color:white;background:transparent;"));
    m_gameOverLabel->setAlignment(Qt::AlignCenter);
    m_gameOverLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    m_pausedLabel = new QLabel(QStringLiteral("PAUSED"), this);
    QFont pfl(QStringLiteral("Times New Roman")); pfl.setBold(true); pfl.setPointSize(42);
    m_pausedLabel->setFont(pfl);
    m_pausedLabel->setStyleSheet(QStringLiteral("color:white;background:rgba(0,0,0,120);"));
    m_pausedLabel->setAlignment(Qt::AlignCenter);
    m_pausedLabel->hide();

    setupSettingsPanel();
    setupHiscorePanel();
    layoutAll();
    refreshUi();
}

// ======================== Settings panel ========================
void SpaceView::setupSettingsPanel() {
    m_settingsPanel = new QWidget(this);
    m_settingsPanel->setObjectName("settingsPanel");
    m_settingsPanel->setStyleSheet(
        "#settingsPanel{"
        "background-color:transparent;"
        "border:1px solid #7a6e61;border-radius:6px;}"
        "#settingsPanel QLabel{color:#2b2b2b;background:transparent;border:none;}"
        + QString("#settingsPanel ") + kSettingSliderStyle);
    m_settingsPanel->hide();

    m_settingsBgLabel = new QLabel(m_settingsPanel);
    m_settingsBgLabel->setScaledContents(true);
    m_settingsBgLabel->setPixmap(loadImg("APPLE_SETUP.png"));
    m_settingsBgLabel->lower();

    auto addRow = [&](const QString& title, int minV, int maxV, int init, QLabel*& tl, QLabel*& vl, QSlider*& sl, const QString& suffix = "") {
        createSettingRow(m_settingsPanel, title, minV, maxV, init, 1, tl, vl, sl);
        applyCnLabelFont(tl); applyValueLabelFont(vl);
        QObject::connect(sl, &QSlider::valueChanged, [vl, suffix](int v) { vl->setText(QString::number(v) + suffix); });
    };
    addRow("敌机数量(1-10):",  1,  10,  5, m_labelEnemyCount,  m_labelEnemyValue,  m_sliderEnemyCount);
    addRow("敌机速度(1-10):",  1,  10,  5, m_labelSpeed,      m_labelSpeedValue,  m_sliderSpeed);
    addRow("升级间隔(5-60):",  5,  60, 20, m_labelUpgrade,     m_labelUpgradeValue, m_sliderUpgrade, "s");

    // Reward checkbox
    m_checkReward = new QPushButton("奖励模式", m_settingsPanel);
    m_checkReward->setCheckable(true);
    m_checkReward->setFlat(true);
    m_checkReward->setCursor(Qt::PointingHandCursor);
    if (!m_checkUnchecked.isNull()) {
        int s = m_checkUnchecked.width();
        m_checkReward->setIconSize(QSize(s, s));
        m_checkReward->setIcon(QIcon(m_checkUnchecked));
        QFont cbf("SimSun"); cbf.setPointSize(14); cbf.setBold(true); m_checkReward->setFont(cbf);
        m_checkReward->setStyleSheet("QPushButton{color:#2b2b2b;background:transparent;text-align:left;padding-left:4px;}");
        m_checkReward->installEventFilter(this);
        connect(m_checkReward, &QPushButton::toggled, this, [this](bool checked) {
            m_checkReward->setIcon(checked ? QIcon(m_checkChecked) : QIcon(m_checkUnchecked));
        });
    }

    // 与 layoutAll 底部三键尺寸同源（同 SaveApple：create 的 QSize == 布局后的控件尺寸）
    const QSize footerBtnSz = spaceSettingsFooterBtnSize(width(), height());
    m_btnSettingsCancel = ButtonIconManager::create(m_settingsPanel, ":/space/images/images/CANCEL.png", footerBtnSz, m_btnIcons);
    m_btnSettingsOk     = ButtonIconManager::create(m_settingsPanel, ":/space/images/images/OK.png", footerBtnSz, m_btnIcons);
    m_btnSettingsDefault = ButtonIconManager::create(m_settingsPanel, ":/space/images/images/DEFAULT.png", footerBtnSz, m_btnIcons);

    connect(m_btnSettingsCancel, &QPushButton::clicked, this, &SpaceView::hideSettings);
    connect(m_btnSettingsDefault, &QPushButton::clicked, this, [this]() {
        const SpaceSettings d;
        m_sliderEnemyCount->setValue(d.maxEnemies);
        m_labelEnemyValue->setText(QString::number(d.maxEnemies));
        m_sliderSpeed->setValue(d.speedLevel);
        m_labelSpeedValue->setText(QString::number(d.speedLevel));
        m_sliderUpgrade->setValue(d.upgradeIntervalSec);
        m_labelUpgradeValue->setText(QString::number(d.upgradeIntervalSec) + "s");
        m_checkReward->setChecked(d.rewardMode);
        m_checkReward->setIcon(d.rewardMode ? QIcon(m_checkChecked) : QIcon(m_checkUnchecked));
    });
    connect(m_btnSettingsOk, &QPushButton::clicked, this, [this]() {
        if (!m_model) {
            hideSettings();
            return;
        }
        SpaceSettings s = m_model->settings();
        s.maxEnemies = m_sliderEnemyCount->value();
        s.speedLevel = m_sliderSpeed->value();
        s.upgradeIntervalSec = m_sliderUpgrade->value();
        s.rewardMode = m_checkReward->isChecked();
        emit settingsApplied(s);
        hideSettings();
    });
}

// ======================== Hiscore panel ========================
void SpaceView::setupHiscorePanel() {
    m_hiscorePanel = new QWidget(this);
    m_hiscorePanel->setObjectName("hiscorePanel");
    m_hiscorePanel->setStyleSheet("#hiscorePanel{background-color:rgba(20,20,40,230);border:2px solid #5a5a7a;border-radius:12px;}");
    m_hiscorePanel->hide();

    m_hiscoreBgLabel = new QLabel(m_hiscorePanel);
    m_hiscoreBgLabel->setScaledContents(true);
    m_hiscoreBgLabel->setPixmap(loadImg("SPACE_HISCORE_BG.png"));

    for (int i = 0; i < 9; ++i) {
        m_hiscoreScores[i] = new QLabel(m_hiscorePanel);
        m_hiscoreScores[i]->setAlignment(Qt::AlignCenter);
        QFont sf("Times New Roman"); sf.setPointSize(16);
        if (i == 0) sf.setBold(true);
        m_hiscoreScores[i]->setFont(sf);
        m_hiscoreScores[i]->setStyleSheet("color:white;background:rgba(255,255,255,30);border:1px solid rgba(255,255,255,80);border-radius:4px;");
    }

    const QSize hiscoreBackSz = spaceHiscoreBackBtnSize(width(), height());
    m_btnHiscoreBack = ButtonIconManager::create(m_hiscorePanel, ":/space/images/images/SPACE_RETURN.png", hiscoreBackSz, m_btnIcons);
    connect(m_btnHiscoreBack, &QPushButton::clicked, this, &SpaceView::hideHiscorePanel);
}

// ======================== Layout（按界面拆分，坐标系见 layout_export 1704×1278）====================

void SpaceView::layoutAll() {
    const int w = width(), h = height();
    if (w <= 0 || h <= 0) return;
    layoutGameView(w, h);
    layoutMainMenu(w, h);
    layoutBattleOverlays(w, h);
    layoutSettingsPanel(w, h);
    layoutHiscorePanel(w, h);
    layoutStarfieldCache(w, h);
}

void SpaceView::layoutStarfieldCache(int w, int h) {
    if (m_stars.isNull() || w <= 0 || h <= 0) {
        m_starPix1x1.clear();
        m_starParticles.clear();
        m_starfieldLayoutSize = QSize();
        m_starfieldCache = QPixmap();
        if (m_sceneStarsItem) m_sceneStarsItem->setPixmap(QPixmap());
        return;
    }
    if (m_starfieldLayoutSize == QSize(w, h) && !m_starParticles.isEmpty()
        && m_starPix1x1.size() == kStarTypeCount) {
        return;
    }

    if (!buildScaledStarVariants(m_stars, 1, &m_starPix1x1)) {
        m_starParticles.clear();
        m_starfieldLayoutSize = QSize();
        m_starfieldCache = QPixmap();
        if (m_sceneStarsItem) m_sceneStarsItem->setPixmap(QPixmap());
        return;
    }

    m_starfieldLayoutSize = QSize(w, h);
    const int count = qMax(1, qMin(100, static_cast<int>(w * h * 0.0028 / 3.0)));
    m_starParticles.resize(count);
    auto* rng = QRandomGenerator::global();
    for (int i = 0; i < count; ++i) {
        StarParticle& s = m_starParticles[i];
        s.x = static_cast<float>(rng->bounded(w));
        s.y = static_cast<float>(rng->bounded(h));
        s.kind = static_cast<quint8>(rng->bounded(kStarTypeCount));
        s.vx = (static_cast<float>(rng->bounded(121)) - 60.0f) * 0.25f;
        s.vy = 25.0f + static_cast<float>(rng->bounded(75));
    }
    paintStarfieldFromParticles();
    if (m_sceneStarsItem) m_sceneStarsItem->setPixmap(m_starfieldCache);
}

void SpaceView::paintStarfieldFromParticles() {
    const int w = m_starfieldLayoutSize.width();
    const int h = m_starfieldLayoutSize.height();
    if (w <= 0 || h <= 0 || m_starPix1x1.size() != kStarTypeCount || m_starParticles.isEmpty()) return;

    if (m_starfieldCache.size() != QSize(w, h)) m_starfieldCache = QPixmap(w, h);
    m_starfieldCache.fill(Qt::transparent);
    QPainter painter(&m_starfieldCache);
    for (const StarParticle& s : m_starParticles) {
        const int xi = qBound(0, static_cast<int>(s.x), w - 1);
        const int yi = qBound(0, static_cast<int>(s.y), h - 1);
        painter.drawPixmap(xi, yi, m_starPix1x1[s.kind % kStarTypeCount]);
    }
}

void SpaceView::layoutGameView(int w, int h) {
    m_gameView->setGeometry(0, 0, w, h);
    m_gameScene->setSceneRect(QRectF(0, 0, w, h));
}

void SpaceView::layoutMainMenu(int w, int h) {
    auto px = [w](int refX) { return w * refX / 1704; };
    auto py = [h](int refY) { return h * refY / 1278; };
    QPushButton* btns[] = {m_btnStart, m_btnHiscore, m_btnOption, m_btnExit};
    constexpr int btnW = 590, btnH = 98;
    for (auto* b : btns) b->setFixedSize(btnW, btnH);
    const int baseX = px(539), baseY = py(609);
    for (int i = 0; i < 4; ++i) btns[i]->move(baseX, baseY + i * btnH);
}

void SpaceView::layoutBattleOverlays(int w, int h) {
    auto px = [w](int refX) { return w * refX / 1704; };
    auto py = [h](int refY) { return h * refY / 1278; };
    auto pw = [w](int refW) { return w * refW / 1704; };
    auto ph = [h](int refH) { return h * refH / 1278; };
    m_gameOverOverlay->setGeometry(0, 0, w, h);
    const int goW = pw(1704), goH = ph(72);
    m_gameOverLabel->setGeometry((w - goW) / 2, (h - goH) / 2, goW, goH);
    m_pausedLabel->setGeometry(px(0), py(0), pw(1704), ph(1278));
}

void SpaceView::layoutSettingsPanel(int w, int h) {
    auto px = [w](int refX) { return w * refX / 1704; };
    auto py = [h](int refY) { return h * refY / 1278; };
    auto pw = [w](int refW) { return w * refW / 1704; };
    auto ph = [h](int refH) { return h * refH / 1278; };

    m_settingsPanel->setGeometry(px(213), py(160), pw(1278), ph(958));
    m_settingsBgLabel->setGeometry(0, 0, m_settingsPanel->width(), m_settingsPanel->height());

    const int spw = m_settingsPanel->width(), sph = m_settingsPanel->height();
    auto spx = [spw](int refX) { return spw * refX / 1278; };
    auto spy = [sph](int refY) { return sph * refY / 958; };
    constexpr int kSettingsContentShiftRef = 52;
    auto spyOpt = [&](int refY) { return spy(qMax(24, refY - kSettingsContentShiftRef)); };

    m_labelEnemyCount->setGeometry(spx(319), spyOpt(325), spw * 312 / 1278, sph * 48 / 958);
    m_labelEnemyValue->setGeometry(spx(1049), spyOpt(325), spw * 72 / 1278, sph * 48 / 958);
    m_sliderEnemyCount->setGeometry(spx(641), spyOpt(335), spw * 398 / 1278, sph * 36 / 958);
    m_labelSpeed->setGeometry(spx(319), spyOpt(413), spw * 312 / 1278, sph * 48 / 958);
    m_labelSpeedValue->setGeometry(spx(1049), spyOpt(413), spw * 72 / 1278, sph * 48 / 958);
    m_sliderSpeed->setGeometry(spx(641), spyOpt(423), spw * 398 / 1278, sph * 36 / 958);
    m_labelUpgrade->setGeometry(spx(319), spyOpt(501), spw * 312 / 1278, sph * 48 / 958);
    m_labelUpgradeValue->setGeometry(spx(1049), spyOpt(501), spw * 72 / 1278, sph * 48 / 958);
    m_sliderUpgrade->setGeometry(spx(641), spyOpt(511), spw * 398 / 1278, sph * 36 / 958);
    m_checkReward->setGeometry(spx(319), spyOpt(589), spw * 312 / 1278, sph * 48 / 958);

    QPixmap cb = loadImg("CHECKBOX_BUTTON.png");
    if (!cb.isNull()) {
        const int part = cb.width() / 4, isz = sph * 48 / 958;
        m_checkUnchecked      = cb.copy(0, 0, part, part).scaled(isz, isz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_checkUncheckedHover = cb.copy(part, 0, part, part).scaled(isz, isz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_checkChecked        = cb.copy(part * 2, 0, part, part).scaled(isz, isz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_checkCheckedHover   = cb.copy(part * 3, 0, part, part).scaled(isz, isz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_checkReward->setIconSize(QSize(isz, isz));
        m_checkReward->setIcon(m_checkReward->isChecked() ? QIcon(m_checkChecked) : QIcon(m_checkUnchecked));
    }

    const int s3bw = spw * 226 / 1278, s3bh = sph * 98 / 958;
    const int cancelX = spx(500), cancelY = spy(813);
    const int s3gap = spw * 31 / 1278;
    const QSize footerIconSz(s3bw, s3bh);
    auto setBtn = [&](QPushButton* b, int x) {
        if (!b) return;
        b->setFixedSize(footerIconSz);
        b->move(x, cancelY);
    };
    setBtn(m_btnSettingsCancel, cancelX);
    setBtn(m_btnSettingsOk, spx(757));
    setBtn(m_btnSettingsDefault, spx(757) + s3bw + s3gap);
    m_btnIcons->registerButton(m_btnSettingsCancel, ":/space/images/images/CANCEL.png", footerIconSz);
    m_btnIcons->registerButton(m_btnSettingsOk, ":/space/images/images/OK.png", footerIconSz);
    m_btnIcons->registerButton(m_btnSettingsDefault, ":/space/images/images/DEFAULT.png", footerIconSz);
}

void SpaceView::layoutHiscorePanel(int w, int h) {
    auto px = [w](int refX) { return w * refX / 1704; };
    auto py = [h](int refY) { return h * refY / 1278; };
    auto pw = [w](int refW) { return w * refW / 1704; };
    auto ph = [h](int refH) { return h * refH / 1278; };

    m_hiscorePanel->setGeometry(px(213), py(160), pw(1278), ph(958));
    m_hiscoreBgLabel->setGeometry(0, 0, m_hiscorePanel->width(), m_hiscorePanel->height());

    const int hpw = m_hiscorePanel->width(), hph = m_hiscorePanel->height();
    const int sx = hpw * 393 / 1278, sw = hpw * 556 / 1278, sh = hph * 54 / 958;
    const int firstY = hph * 246 / 958, lastY = hph * 708 / 958;
    const int step = (lastY - firstY) / 8;
    for (int i = 0; i < 9; ++i)
        m_hiscoreScores[i]->setGeometry(sx, firstY + i * step, sw, sh);
    const int bx = hpw * 867 / 1278, by = hph * 796 / 958, bw = hpw * 332 / 1278, bh = hph * 68 / 958;
    const QSize btnSz(bw, bh);
    m_btnHiscoreBack->setFixedSize(btnSz);
    m_btnHiscoreBack->move(bx, by);
    m_btnIcons->registerButton(m_btnHiscoreBack, ":/space/images/images/SPACE_RETURN.png", btnSz);
}

void SpaceView::resizeEvent(QResizeEvent*) { layoutAll(); }

// ======================== Background paint ========================
void SpaceView::paintEvent(QPaintEvent*) {
    QPainter p(this);
    const bool inMenu = !m_model || m_model->state() == SpaceGameState::Initial
                        || m_model->state() == SpaceGameState::End;
    if (inMenu && !m_menuBg.isNull()) {
        p.drawPixmap(rect(), m_menuBg.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

// ======================== refreshUi ========================
void SpaceView::refreshUi(double gameTickDeltaSec) {
    const bool inMenu = !m_model || m_model->state() == SpaceGameState::Initial
                        || m_model->state() == SpaceGameState::End;
    const bool playing = m_model && m_model->state() == SpaceGameState::Playing;
    const bool paused  = m_model && m_model->state() == SpaceGameState::Paused;
    const bool gameOver = m_model && m_model->state() == SpaceGameState::GameOver;
    const bool inBattle = playing || paused || gameOver;

    // Menu buttons
    m_btnStart->setVisible(inMenu);
    m_btnHiscore->setVisible(inMenu);
    m_btnOption->setVisible(inMenu);
    m_btnExit->setVisible(inMenu);

    m_gameOverOverlay->setVisible(gameOver);
    if (gameOver) m_gameOverOverlay->raise();

    m_pausedLabel->setVisible(paused);
    if (paused) m_pausedLabel->raise();

    // Overlay panels hidden when leaving menu
    if (!inMenu) {
        m_settingsPanel->hide();
        m_hiscorePanel->hide();
    }

    // Game view & scene
    if (inBattle) {
        m_gameView->show();
        if (!m_staticSceneCreated) {
            createStaticScene();
            m_staticSceneCreated = true;
        }
        updateDynamicScene(gameTickDeltaSec);
    } else {
        m_gameView->hide();
        if (m_staticSceneCreated) {
            const auto staticChildren = m_staticGroup->childItems();
            for (auto* item : staticChildren) delete item;
            m_sceneLifeFill = nullptr;
            m_sceneStarsItem = nullptr;
            m_sceneScoreText = nullptr;
            m_sceneTimeText = nullptr;
            m_staticSceneCreated = false;
        }
    }
}

// ======================== Panel show/hide ========================
void SpaceView::showSettings() {
    if (!m_model) return;
    m_sliderEnemyCount->setValue(m_model->settings().maxEnemies);
    m_labelEnemyValue->setText(QString::number(m_model->settings().maxEnemies));
    m_sliderSpeed->setValue(m_model->settings().speedLevel);
    m_labelSpeedValue->setText(QString::number(m_model->settings().speedLevel));
    m_sliderUpgrade->setValue(m_model->settings().upgradeIntervalSec);
    m_labelUpgradeValue->setText(QString::number(m_model->settings().upgradeIntervalSec) + "s");
    m_checkReward->setChecked(m_model->settings().rewardMode);
    m_checkReward->setIcon(m_model->settings().rewardMode ? QIcon(m_checkChecked) : QIcon(m_checkUnchecked));
    m_settingsPanel->show();
    m_settingsPanel->raise();
}

void SpaceView::hideSettings() { m_settingsPanel->hide(); }

void SpaceView::showHiscorePanel() {
    for (int i = 0; i < 9; ++i) {
        m_hiscoreScores[i]->setText(i < m_highScores.size() ? QString::number(m_highScores[i]) : "-----");
    }
    m_hiscorePanel->show();
    m_hiscorePanel->raise();
}
void SpaceView::hideHiscorePanel() { m_hiscorePanel->hide(); }

// ======================== Event filter (checkbox only; buttons via ButtonIconManager) ========================
bool SpaceView::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_gameOverOverlay && event->type() == QEvent::MouseButtonPress) {
        emit dismissGameOver();
        return true;
    }
    // 4-state checkbox
    auto* btn = qobject_cast<QPushButton*>(watched);
    if (btn == m_checkReward && !m_checkUnchecked.isNull()) {
        if (event->type() == QEvent::Enter) {
            btn->setIcon(btn->isChecked() ? QIcon(m_checkCheckedHover) : QIcon(m_checkUncheckedHover));
        } else if (event->type() == QEvent::Leave) {
            btn->setIcon(btn->isChecked() ? QIcon(m_checkChecked) : QIcon(m_checkUnchecked));
        }
        return false;
    }
    return false;
}

// ======================== Keyboard ========================
void SpaceView::keyPressEvent(QKeyEvent* e) {
    if (e->isAutoRepeat()) return;
    if (e->key() == Qt::Key_Space) {
        if (m_model && (m_model->state() == SpaceGameState::Playing
                        || m_model->state() == SpaceGameState::Paused)) {
            emit pauseClicked();
            return;
        }
    }
    // Esc：从「玩 / 暂停 / 游戏结束」回主菜单，与鼠标点穿 Game Over 遮罩语义一致，统一走 returnToMenu
    if (e->key() == Qt::Key_Escape) {
        if (m_model) {
            const SpaceGameState st = m_model->state();
            if (st == SpaceGameState::Playing || st == SpaceGameState::Paused
                || st == SpaceGameState::GameOver) {
                emit returnToMenu();
                return;
            }
        }
    }
    if (!m_model || m_model->state() != SpaceGameState::Playing) {
        QWidget::keyPressEvent(e);
        return;
    }
    if (e->key() >= Qt::Key_A && e->key() <= Qt::Key_Z) {
        const QChar keyLetter = QChar('A' + (e->key() - Qt::Key_A));
        // 敌机字幕按大写 A–Z；控制器里与 e.letter 比较时会 toUpper，此处用大写即可。
        emit letterPressed(keyLetter);
        // 奖励词路径独立：仅开启奖励模式时才发信号（onRewardTyped 内仍按 CaseInsensitive 比对）
        if (m_model->settings().rewardMode)
            emit rewardCharTyped(keyLetter.toLower());
        return;
    }
    QWidget::keyPressEvent(e);
}


// ======================== Game scene ========================
void SpaceView::createStaticScene() {
    if (!m_staticGroup) return;
    int w = width(), h = height();
    if (w <= 0 || h <= 0) return;

    // Background
    if (!m_bg.isNull()) {
        auto* bg = new QGraphicsPixmapItem(m_bg.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        bg->setPos(0, 0);
        bg->setZValue(0);
        m_staticGroup->addToGroup(bg);
    }
    m_sceneStarsItem = nullptr;
    if (!m_starfieldCache.isNull()) {
        m_sceneStarsItem = new QGraphicsPixmapItem(m_starfieldCache);
        m_sceneStarsItem->setPos(0, 0);
        m_sceneStarsItem->setZValue(1);
        m_staticGroup->addToGroup(m_sceneStarsItem);
    }
    // HUD labels（z 高于星空，避免被星星盖住）
    int hudLeft = 10, hudTop = 10;
    int rowH = 54;
    int lifeY = hudTop, scoreY = lifeY + rowH, timeY = scoreY + rowH;
    int labelW = 86, labelH = 44;
    if (!m_labelLife.isNull())  { auto* i = new QGraphicsPixmapItem(m_labelLife.scaled(labelW, labelH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));  i->setPos(hudLeft, lifeY);  i->setZValue(2); m_staticGroup->addToGroup(i); }
    if (!m_labelScore.isNull()) { auto* i = new QGraphicsPixmapItem(m_labelScore.scaled(labelW, labelH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)); i->setPos(hudLeft, scoreY); i->setZValue(2); m_staticGroup->addToGroup(i); }
    if (!m_labelTime.isNull())  { auto* i = new QGraphicsPixmapItem(m_labelTime.scaled(labelW, labelH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));  i->setPos(hudLeft, timeY);  i->setZValue(2); m_staticGroup->addToGroup(i); }

    // Life bar background
    int barX = hudLeft + 100, barY = lifeY + 6;
    int barW = m_lifeBar.isNull() ? 181 : m_lifeBar.width();
    int barH = m_lifeBar.isNull() ? 22 : m_lifeBar.height();
    if (!m_lifeBar.isNull()) {
        auto* lifeBg = new QGraphicsPixmapItem(m_lifeBar.scaled(barW * 2, barH * 2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        lifeBg->setPos(barX, barY);
        lifeBg->setZValue(2);
        m_staticGroup->addToGroup(lifeBg);
    }
    // Life bar fill — created once, updated per frame
    m_sceneLifeFill = new QGraphicsPixmapItem();
    m_sceneLifeFill->setPos(barX, barY);
    m_sceneLifeFill->setZValue(2);
    m_staticGroup->addToGroup(m_sceneLifeFill);

    // Score / Time text
    QFont hudFont("Times New Roman"); hudFont.setBold(true); hudFont.setPixelSize(36);
    m_sceneScoreText = new QGraphicsTextItem();
    m_sceneScoreText->setFont(hudFont);
    m_sceneScoreText->setDefaultTextColor(Qt::white); m_sceneScoreText->setPos(barX, scoreY - 4);
    m_sceneScoreText->setZValue(2);
    m_staticGroup->addToGroup(m_sceneScoreText);
    m_sceneTimeText = new QGraphicsTextItem();
    m_sceneTimeText->setFont(hudFont);
    m_sceneTimeText->setDefaultTextColor(Qt::white); m_sceneTimeText->setPos(barX, timeY - 4);
    m_sceneTimeText->setZValue(2);
    m_staticGroup->addToGroup(m_sceneTimeText);
}

void SpaceView::updateDynamicScene(double gameTickDeltaSec) {
    if (!m_dynamicGroup || !m_model) return;
    const auto children = m_dynamicGroup->childItems();
    for (auto* item : children) delete item;

    int w = width(), h = height();
    if (w <= 0 || h <= 0) return;

    if (m_model->state() == SpaceGameState::Playing && !m_starParticles.isEmpty()
        && m_starfieldLayoutSize == QSize(w, h)) {
        const double dt = (gameTickDeltaSec > 0.0) ? gameTickDeltaSec : 0.016;
        for (StarParticle& s : m_starParticles) {
            s.x += s.vx * static_cast<float>(dt);
            s.y += s.vy * static_cast<float>(dt);
            while (s.x < 0) s.x += static_cast<float>(w);
            while (s.x >= static_cast<float>(w)) s.x -= static_cast<float>(w);
            while (s.y < 0) s.y += static_cast<float>(h);
            while (s.y >= static_cast<float>(h)) s.y -= static_cast<float>(h);
        }
        paintStarfieldFromParticles();
        if (m_sceneStarsItem) m_sceneStarsItem->setPixmap(m_starfieldCache);
    }

    constexpr int kDrawShip = 160;
    constexpr int kDrawEnemy = 128;

    // Ship
    QPointF ship = m_model->shipPos();
    if (!m_ship.isNull()) {
        const QPixmap shipCell = cropSpriteFrame34(m_ship, m_model->shipAnimFrame());
        if (!shipCell.isNull()) {
            auto* si = new QGraphicsPixmapItem(
                shipCell.scaled(kDrawShip, kDrawShip, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            si->setPos(ship.x(), ship.y());
            m_dynamicGroup->addToGroup(si);
        }
    }
    // Enemies
    for (const auto& e : m_model->enemies()) {
        if (!e.alive) continue;
        const int sz = kDrawEnemy;
        if (e.exploding) {
            if (!m_explosionFull.isNull()) {
                int fw = m_explosionFull.width() / 3, fh = m_explosionFull.height() / 3;
                int col = qMin(e.explosionFrame, 8) % 3;
                int row = qMin(e.explosionFrame, 8) / 3;
                QPixmap frame = m_explosionFull.copy(col * fw, row * fh, fw, fh);
                auto* ex = new QGraphicsPixmapItem(frame.scaled(sz, sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                ex->setPos(e.pos);
                m_dynamicGroup->addToGroup(ex);
            }
            continue;
        }
        if (e.type == EnemyType::Meteor) {
            ClusterLayoutProfile meteorProf = m_enemyClusterTemplate;
            meteorProf.captionFromCarrierY = m_meteorCaptionFromCarrierY;
            EnemyCaptionLetterFactory::addGraphicsCluster(
                m_dynamicGroup, e.pos, m_meteorFull, e.animFrame, m_captionBack, e.letter, meteorProf);
        } else {
            EnemyCaptionLetterFactory::addGraphicsCluster(
                m_dynamicGroup, e.pos, m_enemyA, e.animFrame, m_captionBack, e.letter, m_enemyClusterTemplate);
        }
    }
    // Bullets
    for (const auto& b : m_model->bullets()) {
        if (!b.alive) continue;
        if (!m_bomb.isNull()) {
            auto* bi = new QGraphicsPixmapItem(m_bomb.scaled(12, 12, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            bi->setPos(b.pos.x() - 6, b.pos.y() - 6);
            m_dynamicGroup->addToGroup(bi);
        } else {
            auto* bi = new QGraphicsEllipseItem(b.pos.x() - 4, b.pos.y() - 4, 8, 8);
            bi->setPen(Qt::NoPen); bi->setBrush(QBrush(QColor(255, 220, 120)));
            m_dynamicGroup->addToGroup(bi);
        }
    }
    // Reward word
    if (m_model->settings().rewardMode && m_model->rewardWord().active) {
        const QString& word = m_model->rewardWord().word;
        const QString& buf = m_model->typedRewardBuffer();
        QString display;
        for (int i = 0; i < word.length(); ++i) {
            QChar wc = word[i];
            if (i < buf.length() && wc.toLower() == buf[i].toLower()) {
                display += QString("<span style='color:#33ff66;'>%1</span>").arg(wc);
            } else {
                display += QString("<span style='color:#ffdd88;'>%1</span>").arg(wc);
            }
        }
        auto* rt = new QGraphicsTextItem();
        rt->setHtml(display);
        QFont rf = rt->font(); rf.setFamily("Times New Roman"); rf.setBold(true); rf.setPointSize(16); rt->setFont(rf);
        rt->setPos(m_model->rewardWord().pos);
        m_dynamicGroup->addToGroup(rt);
    }

    // Update persistent HUD values each frame
    constexpr int maxLife = 18;
    const int life = qBound(0, m_model->life(), maxLife);
    int barW = m_lifeBar.isNull() ? 181 : m_lifeBar.width();
    int barH = m_lifeBar.isNull() ? 22 : m_lifeBar.height();
    if (m_sceneLifeFill) {
        if (life > 0 && !m_lifeOver.isNull()) {
            int fillW = static_cast<int>(barW * 2 * (static_cast<qreal>(life) / maxLife));
            m_sceneLifeFill->setPixmap(m_lifeOver.scaled(qMax(1, fillW), barH * 2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            m_sceneLifeFill->setVisible(true);
        } else {
            m_sceneLifeFill->setVisible(false);
        }
    }
    if (m_sceneScoreText) m_sceneScoreText->setPlainText(QString::number(m_model->score()));
    if (m_sceneTimeText) m_sceneTimeText->setPlainText(QString::number(static_cast<int>(m_model->elapsedSec())));
}

