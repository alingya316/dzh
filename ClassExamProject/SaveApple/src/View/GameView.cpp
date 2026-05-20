#include "GameView.h"

#include "Config/GameConfig.h"
#include "Model/GameData.h"
#include "System/AppleAudio.h"

#include "exam/ButtonIconManager.h"
#include "exam/SettingsHelper.h"

#include <QIcon>
#include <QEvent>
#include <QFontMetrics>
#include <QLabel>
#include <QKeyEvent>
#include <QPainter>
#include <QSlider>

namespace {
// 统一维护 UI/布局参数，后续调参只改这里（像素单位 px）。
constexpr int kMinWindowWidth = 960;   // 主窗口最小宽度。
constexpr int kMinWindowHeight = 600;  // 主窗口最小高度。

// 图标缩放参数：图标尺寸与按钮尺寸保持 1:1。
constexpr int kMinIconSize = 16;  // 图标最小尺寸下限，避免显示过小。

// 左下角四个功能按钮参数（位置与尺寸分别独立设置，便于单独微调）。
constexpr int kStartButtonWidth = 107;  // 开始按钮宽度。
constexpr int kStartButtonHeight = 83;  // 开始按钮高度。
constexpr int kStartButtonX = 442;      // 开始按钮左上角 X 坐标。
constexpr int kStartButtonY = 1028;     // 开始按钮左上角 Y 坐标。

constexpr int kPauseButtonWidth = 104;  // 暂停按钮宽度。
constexpr int kPauseButtonHeight = 86;  // 暂停按钮高度。
constexpr int kPauseButtonX = 319;      // 暂停按钮左上角 X 坐标。
constexpr int kPauseButtonY = 1093;     // 暂停按钮左上角 Y 坐标。

constexpr int kEndButtonWidth = 112;    // 结束按钮宽度。
constexpr int kEndButtonHeight = 87;    // 结束按钮高度。
constexpr int kEndButtonX = 405;        // 结束按钮左上角 X 坐标。
constexpr int kEndButtonY = 1173;       // 结束按钮左上角 Y 坐标。

constexpr int kSetupButtonWidth = 128;  // 设置按钮宽度。
constexpr int kSetupButtonHeight = 107; // 设置按钮高度。
constexpr int kSetupButtonX = 536;      // 设置按钮左上角 X 坐标。
constexpr int kSetupButtonY = 1119;     // 设置按钮左上角 Y 坐标。

// 退出按钮参数：位置与尺寸（统一使用绝对坐标语义）。
constexpr int kExitButtonWidth = 288;   // 退出按钮宽度。
constexpr int kExitButtonHeight = 192;  // 退出按钮高度。
constexpr int kExitButtonX = 20;        // 退出按钮左上角 X 坐标。
constexpr int kExitButtonY = 1065;      // 退出按钮左上角 Y 坐标。

// 设置面板参数：最大占窗口比例。
constexpr qreal kSettingPanelMaxScale = 0.75;  // 设置面板占主窗口比例。

// 设置项范围参数。
constexpr int kSpeedMin = 1;                    // 下落速度最小值。
constexpr int kSpeedMax = 10;                   // 下落速度最大值。
constexpr int kMaxAppleMin = 1;                 // 同屏苹果数量最小值。
constexpr int kMaxAppleMax = 5;                 // 同屏苹果数量最大值。
constexpr int kTargetMin = 5;                   // 目标分数最小值。
constexpr int kTargetMax = 100;                 // 目标分数最大值。
constexpr int kTargetStep = 5;                  // 目标分数滑条步进。
constexpr int kSettingSliderWidth = 140;        // 设置滑条基准宽度（保留参数）。
constexpr int kDefaultSpeed = 4;                // 默认下落速度。
constexpr int kDefaultMaxApple = 3;             // 默认同屏苹果数。
constexpr int kDefaultTarget = 15;              // 默认目标分数。
constexpr qreal kSettingContentXRatio = 0.44;   // 设置内容区域起始 X 比例。
constexpr int kSettingValueWidth = 72;          // 右侧数值标签宽度。
constexpr int kSettingRowHeight = 48;           // 每行控件高度。
constexpr int kSettingRowGap = 40;              // 相邻两行垂直间距。
constexpr qreal kSettingFirstRowYRatio = 0.34;  // 第一行 Y 位置占面板高度比例。
constexpr int kSettingSliderOffsetX = 6;        // 滑条 X 微调偏移。
constexpr int kSettingSliderOffsetY = 10;       // 滑条 Y 微调偏移。
constexpr int kSettingSliderHeightAdjust = 14;  // 滑条高度修正量。
constexpr int kSettingBetweenGap = 10;          // 标题/滑条/数值之间间隔。
constexpr qreal kSettingLabelAnchorXRatio = 0.25;      // 左侧标题锚点 X 比例。
constexpr qreal kSettingLabelMinWidthRatio = 0.24;     // 标题最小宽度占比。
constexpr int kSettingLabelWidthPadding = 6;           // 标题宽度补偿。
constexpr int kSettingSliderMinWidth = 60;             // 滑条最小宽度。
constexpr int kSettingPanelRightPadding = 24;          // 面板右内边距。
constexpr int kSettingLabelFontSize = 14;              // 设置标题字号。
constexpr int kSettingValueFontSize = 14;              // 设置数值字号。
// 设置面板底部三个功能按钮参数（可独立调节位置和尺寸）。
constexpr int kSettingCancelButtonX = 635;      // 取消按钮左上角 X 坐标。
constexpr int kSettingCancelButtonY = 800;      // 取消按钮左上角 Y 坐标。
constexpr int kSettingCancelButtonWidth = 278;  // 取消按钮宽度。
constexpr int kSettingCancelButtonHeight = 111; // 取消按钮高度。

constexpr int kSettingOkButtonX = 960;      // 确定按钮左上角 X 坐标。
constexpr int kSettingOkButtonY = 799;      // 确定按钮左上角 Y 坐标。
constexpr int kSettingOkButtonWidth = 280;  // 确定按钮宽度。
constexpr int kSettingOkButtonHeight = 110; // 确定按钮高度。

constexpr int kSettingDefaultButtonX = 1288;      // 默认按钮左上角 X 坐标。
constexpr int kSettingDefaultButtonY = 801;       // 默认按钮左上角 Y 坐标。
constexpr int kSettingDefaultButtonWidth = 280;   // 默认按钮宽度。
constexpr int kSettingDefaultButtonHeight = 109;  // 默认按钮高度。

// 篮子/水果兜底尺寸参数（资源未加载时使用）。
constexpr int kFallbackBasketWidth = 120;  // 篮子贴图缺失时兜底宽度。
constexpr int kFallbackBasketHeight = 48;  // 篮子贴图缺失时兜底高度。
constexpr int kBasketRightMargin = 16;     // 篮子距窗口右边距。
constexpr int kBasketBottomMargin = 16;    // 篮子距窗口下边距。
constexpr int kFallbackFruitSize = 28;     // 水果贴图缺失时兜底尺寸。
constexpr qreal kFruitSpriteScale = 1.5;   // 水果渲染缩放倍率。

// 左上 HUD 信息框参数。
constexpr int kHudX = 12;   // HUD 左上角 X 坐标。
constexpr int kHudY = 10;   // HUD 左上角 Y 坐标。
constexpr int kHudW = 250;  // HUD 宽度。
constexpr int kHudH = 192;  // HUD 高度。
// 过关提示参数。
constexpr int kPassedTextPointSize = 28;              // 过关提示字号。
const QColor kPassedTextColor(255, 236, 140);         // 过关提示颜色。
const QString kPassedText = QStringLiteral("恭喜过关\n点击开始继续"); // 过关提示文案。
const char* kLatinFontFamily = "Times New Roman";     // 英文/数字字体。
const char* kChineseFontFamily = "SimSun";            // 中文字体（宋体）。
}  // namespace

GameView::GameView(GameData* data, QWidget* parent)
    : QWidget(parent),
      m_data(data),
      m_btnIcons(new ButtonIconManager(this)),
      m_startButton(nullptr),
      m_pauseButton(nullptr),
      m_endButton(nullptr),
      m_exitButton(nullptr),
      m_setupButton(nullptr),
      m_settingPanel(nullptr),
      m_settingBgLabel(nullptr),
      m_settingCancelButton(nullptr),
      m_settingOkButton(nullptr),
      m_settingDefaultButton(nullptr),
      m_speedLabel(nullptr),
      m_speedValueLabel(nullptr),
      m_speedSlider(nullptr),
      m_maxAppleLabel(nullptr),
      m_maxAppleValueLabel(nullptr),
      m_maxAppleSlider(nullptr),
      m_targetLabel(nullptr),
      m_targetValueLabel(nullptr),
      m_targetSlider(nullptr) {
    setFocusPolicy(Qt::StrongFocus);
    m_btnIcons->setAudioCallback([](bool hover) {
        AppleAudio::instance().play(hover ? AppleAudioEvent::ButtonHover
                                          : AppleAudioEvent::ButtonClick);
    });
    setMinimumSize(kMinWindowWidth, kMinWindowHeight);
    loadAssets();
    setupUi();
    if (m_data) {
        m_data->attach(this);
    }
}

GameView::~GameView() {
    if (m_data) {
        m_data->detach(this);
    }
}

void GameView::setMainControlState(bool running) {
    if (m_startButton) {
        m_startButton->setEnabled(!running);
    }
    if (m_pauseButton) {
        m_pauseButton->setEnabled(running);
    }
    if (m_endButton) {
        m_endButton->setEnabled(running);
    }
    if (m_setupButton) {
        // 按要求：仅未开始时可打开设置面板。
        m_setupButton->setEnabled(!running);
    }
}

void GameView::loadAssets() {
    const GameConfig& cfg = GameConfig::instance();
    m_background = QPixmap(cfg.imagePath("APPLE_BACKGROUND.png"));
    m_goodApple = QPixmap(cfg.imagePath("APPLE_NORMAL.png"));
    m_smallApple = QPixmap(cfg.imagePath("APPLE_SMALL.png"));
    m_badApple = QPixmap(cfg.imagePath("APPLE_BAD.png"));
    m_basket = QPixmap(cfg.imagePath("APPLE_BASKET.png"));
}

void GameView::setupUi() {
    setupMainUi();
    setupSettingUi();

    // 初始化完成后先做一次控件布局。
    resizeButtons();
    setMainControlState(false);
}

void GameView::setupMainUi() {
    const GameConfig& cfg = GameConfig::instance();

    // 1) 创建底部主功能按钮（开始/暂停/结束/设置/退出）。
    m_startButton = ButtonIconManager::create(this, cfg.commonImagePath("PUBLIC_START.png"), QSize(kStartButtonWidth, kStartButtonHeight), m_btnIcons);
    m_pauseButton = ButtonIconManager::create(this, cfg.commonImagePath("PUBLIC_PAUSE.png"), QSize(kPauseButtonWidth, kPauseButtonHeight), m_btnIcons);
    m_endButton = ButtonIconManager::create(this, cfg.commonImagePath("PUBLIC_END.png"), QSize(kEndButtonWidth, kEndButtonHeight), m_btnIcons);
    m_exitButton = ButtonIconManager::create(this, cfg.commonImagePath("PUBLIC_EXIT.png"), QSize(kExitButtonWidth, kExitButtonHeight), m_btnIcons);
    m_setupButton = ButtonIconManager::create(this, cfg.commonImagePath("PUBLIC_SETUP.png"), QSize(kSetupButtonWidth, kSetupButtonHeight), m_btnIcons);
    

    // 主功能按钮点击 -> 转发为 View 信号，交给 Controller 处理业务逻辑。
    connect(m_startButton, &QPushButton::clicked, this, &GameView::startClicked);
    connect(m_pauseButton, &QPushButton::clicked, this, &GameView::pauseClicked);
    connect(m_endButton, &QPushButton::clicked, this, &GameView::endClicked);
    connect(m_exitButton, &QPushButton::clicked, this, &GameView::exitClicked);
    connect(m_setupButton, &QPushButton::clicked, this, &GameView::showSettingPanel);
}

void GameView::setupSettingUi() {
    const GameConfig& cfg = GameConfig::instance();

    // 2) 创建设置子窗口（独立顶层窗口，初始隐藏），背景使用 APPLE_SETUP.png。
    m_settingPanel = new QWidget(this, Qt::Dialog | Qt::FramelessWindowHint);
    m_settingPanel->setObjectName("settingPanel");
    m_settingPanel->setVisible(false);
    m_settingPanel->setWindowModality(Qt::ApplicationModal);
    m_settingPanel->setAttribute(Qt::WA_StyledBackground, true);
    m_settingPanel->setStyleSheet(
        "#settingPanel{"
        "background-color:transparent;"
        "border:1px solid #7a6e61;border-radius:6px;}"
        "#settingPanel QLabel,#settingPanel QCheckBox{color:#2b2b2b;background:transparent;border:none;}"
        "#settingPanel QSlider{background:transparent;min-height:36px;}"
        "#settingPanel QSlider::groove:horizontal{height:10px;background:rgba(0,0,0,110);border-radius:5px;}"
        "#settingPanel QSlider::handle:horizontal{width:24px;margin:-8px 0;background:#f3f3f3;border:1px solid #777;border-radius:12px;}");

    m_settingBgLabel = new QLabel(m_settingPanel);
    m_settingBgLabel->setScaledContents(true);
    const QPixmap settingBg(cfg.imagePath("APPLE_SETUP.png"));
    m_settingBgLabel->setPixmap(settingBg);
    m_settingBgLabel->lower();

    createSettingRow(m_settingPanel, QStringLiteral("速度调节(1-10):"),
                     kSpeedMin, kSpeedMax, kDefaultSpeed, 1,
                     m_speedLabel, m_speedValueLabel, m_speedSlider);
    createSettingRow(m_settingPanel, QStringLiteral("数量调节(同屏1-5):"),
                     kMaxAppleMin, kMaxAppleMax, kDefaultMaxApple, 1,
                     m_maxAppleLabel, m_maxAppleValueLabel, m_maxAppleSlider);
    createSettingRow(m_settingPanel, QStringLiteral("目标调节(过关数量):"),
                     kTargetMin, kTargetMax, kDefaultTarget, kTargetStep,
                     m_targetLabel, m_targetValueLabel, m_targetSlider);

    applyCnLabelFont(m_speedLabel);
    applyCnLabelFont(m_maxAppleLabel);
    applyCnLabelFont(m_targetLabel);

    applyLatinValueFont(m_speedValueLabel);
    applyLatinValueFont(m_maxAppleValueLabel);
    applyLatinValueFont(m_targetValueLabel);

    m_settingCancelButton = ButtonIconManager::create(
        m_settingPanel, cfg.commonImagePath("CANCEL.png"),
        QSize(kSettingCancelButtonWidth, kSettingCancelButtonHeight), m_btnIcons);
    m_settingOkButton = ButtonIconManager::create(
        m_settingPanel, cfg.commonImagePath("OK.png"),
        QSize(kSettingOkButtonWidth, kSettingOkButtonHeight), m_btnIcons);
    m_settingDefaultButton = ButtonIconManager::create(
        m_settingPanel, cfg.commonImagePath("DEFAULT.png"),
        QSize(kSettingDefaultButtonWidth, kSettingDefaultButtonHeight), m_btnIcons);

    // 3) 设置面板内部交互信号。
    connect(m_settingCancelButton, &QPushButton::clicked, this, &GameView::hideSettingPanel);
    connect(m_settingOkButton, &QPushButton::clicked, this, [this]() {
        // 仅在点击“确定”时提交设置到 Controller/Model。
        if (m_speedSlider) {
            emit speedChanged(m_speedSlider->value());
        }
        if (m_maxAppleSlider) {
            emit maxAppleChanged(m_maxAppleSlider->value());
        }
        if (m_targetSlider) {
            const int snapped = (m_targetSlider->value() / kTargetStep) * kTargetStep;
            emit targetChanged(snapped);
        }
        hideSettingPanel();
    });
    connect(m_settingDefaultButton, &QPushButton::clicked, this, [this]() {
        // 恢复默认设置，并通过已有信号链同步到 Controller/Model。
        if (m_speedSlider) {
            m_speedSlider->setValue(kDefaultSpeed);
        }
        if (m_maxAppleSlider) {
            m_maxAppleSlider->setValue(kDefaultMaxApple);
        }
        if (m_targetSlider) {
            m_targetSlider->setValue(kDefaultTarget);
        }
    });

    connect(m_speedSlider, &QSlider::valueChanged, this, [this](int value) {
        if (m_speedValueLabel) {
            m_speedValueLabel->setText(QString::number(value));
        }
    });
    connect(m_maxAppleSlider, &QSlider::valueChanged, this, [this](int value) {
        if (m_maxAppleValueLabel) {
            m_maxAppleValueLabel->setText(QString::number(value));
        }
    });
    connect(m_targetSlider, &QSlider::valueChanged, this, [this](int value) {
        const int snapped = (value / kTargetStep) * kTargetStep;
        if (snapped != value && m_targetSlider) {
            m_targetSlider->blockSignals(true);
            m_targetSlider->setValue(snapped);
            m_targetSlider->blockSignals(false);
        }
        if (m_targetValueLabel) {
            m_targetValueLabel->setText(QString::number(snapped));
        }
    });
}

void GameView::applyLatinValueFont(QLabel* label) {
    if (!label) {
        return;
    }
    QFont f = label->font();
    f.setFamily(kLatinFontFamily);
    f.setPointSize(kSettingValueFontSize);
    f.setBold(true);
    label->setFont(f);
    label->setStyleSheet("color:#2b2b2b;background:transparent;border:none;");
}

void GameView::resizeButtons() {
    applyDefaultMainButtonLayout();
    if (m_settingPanel) {
        // 设置窗口按比例缩放，并保持在主窗口中居中。
        const int panelW = static_cast<int>(width() * kSettingPanelMaxScale);
        const int panelH = static_cast<int>(height() * kSettingPanelMaxScale);
        const int panelX = static_cast<int>(width() * (1.0 - kSettingPanelMaxScale) * 0.5);
        const int panelY = static_cast<int>(height() * (1.0 - kSettingPanelMaxScale) * 0.5);
        const QPoint globalTopLeft = mapToGlobal(QPoint(panelX, panelY));
        m_settingPanel->setGeometry(globalTopLeft.x(), globalTopLeft.y(), panelW, panelH);
        if (m_settingBgLabel) {
            m_settingBgLabel->setGeometry(0, 0, panelW, panelH);
        }

        // 设置子窗口内控件：逐个绝对定位。
        const int labelX = static_cast<int>(panelW * kSettingLabelAnchorXRatio);
        int labelW = static_cast<int>(panelW * kSettingLabelMinWidthRatio);
        if (m_speedLabel || m_maxAppleLabel || m_targetLabel) {
            QFontMetrics fm(font());
            if (m_speedLabel) {
                labelW = qMax(labelW, fm.horizontalAdvance(m_speedLabel->text()));
            }
            if (m_maxAppleLabel) {
                labelW = qMax(labelW, fm.horizontalAdvance(m_maxAppleLabel->text()));
            }
            if (m_targetLabel) {
                labelW = qMax(labelW, fm.horizontalAdvance(m_targetLabel->text()));
            }
            labelW += kSettingLabelWidthPadding;
        }
        const int valueW = kSettingValueWidth;
        const int rowH = kSettingRowHeight;
        const int rowGap = kSettingRowGap;
        const int row1Y = static_cast<int>(panelH * kSettingFirstRowYRatio);
        const int row2Y = row1Y + rowH + rowGap;
        const int row3Y = row2Y + rowH + rowGap;
        const int sliderBaseW =
            qMax(kSettingSliderMinWidth,
                 panelW - labelX - labelW - valueW - kSettingBetweenGap * 2 - kSettingPanelRightPadding);
        const int sliderW = qMax(kSettingSliderMinWidth, sliderBaseW * 3 / 4);

        if (m_speedLabel) {
            m_speedLabel->setGeometry(labelX, row1Y, labelW, rowH);
        }
        if (m_speedSlider) {
            m_speedSlider->setGeometry(labelX + labelW + kSettingBetweenGap,
                                       row1Y + kSettingSliderOffsetY,
                                       sliderW,
                                       rowH - kSettingSliderHeightAdjust);
        }
        if (m_speedValueLabel) {
            m_speedValueLabel->setGeometry(labelX + labelW + kSettingBetweenGap + sliderW + kSettingBetweenGap,
                                           row1Y,
                                           valueW,
                                           rowH);
        }

        if (m_maxAppleLabel) {
            m_maxAppleLabel->setGeometry(labelX, row2Y, labelW, rowH);
        }
        if (m_maxAppleSlider) {
            m_maxAppleSlider->setGeometry(labelX + labelW + kSettingBetweenGap,
                                          row2Y + kSettingSliderOffsetY,
                                          sliderW,
                                          rowH - kSettingSliderHeightAdjust);
        }
        if (m_maxAppleValueLabel) {
            m_maxAppleValueLabel->setGeometry(labelX + labelW + kSettingBetweenGap + sliderW + kSettingBetweenGap,
                                              row2Y,
                                              valueW,
                                              rowH);
        }

        if (m_targetLabel) {
            m_targetLabel->setGeometry(labelX, row3Y, labelW, rowH);
        }
        if (m_targetSlider) {
            m_targetSlider->setGeometry(labelX + labelW + kSettingBetweenGap,
                                        row3Y + kSettingSliderOffsetY,
                                        sliderW,
                                        rowH - kSettingSliderHeightAdjust);
        }
        if (m_targetValueLabel) {
            m_targetValueLabel->setGeometry(labelX + labelW + kSettingBetweenGap + sliderW + kSettingBetweenGap,
                                            row3Y,
                                            valueW,
                                            rowH);
        }


        if (m_settingCancelButton) {
            m_settingCancelButton->setGeometry(
                kSettingCancelButtonX, kSettingCancelButtonY, kSettingCancelButtonWidth, kSettingCancelButtonHeight);
        }
        if (m_settingOkButton) {
            m_settingOkButton->setGeometry(
                kSettingOkButtonX, kSettingOkButtonY, kSettingOkButtonWidth, kSettingOkButtonHeight);
        }
        if (m_settingDefaultButton) {
            m_settingDefaultButton->setGeometry(kSettingDefaultButtonX,
                                                kSettingDefaultButtonY,
                                                kSettingDefaultButtonWidth,
                                                kSettingDefaultButtonHeight);
        }
    }
}

void GameView::applyDefaultMainButtonLayout() {
    if (m_startButton) {
        m_startButton->setGeometry(kStartButtonX, kStartButtonY, kStartButtonWidth, kStartButtonHeight);
    }
    if (m_pauseButton) {
        m_pauseButton->setGeometry(kPauseButtonX, kPauseButtonY, kPauseButtonWidth, kPauseButtonHeight);
    }
    if (m_endButton) {
        m_endButton->setGeometry(kEndButtonX, kEndButtonY, kEndButtonWidth, kEndButtonHeight);
    }
    if (m_setupButton) {
        m_setupButton->setGeometry(kSetupButtonX, kSetupButtonY, kSetupButtonWidth, kSetupButtonHeight);
    }
    if (m_exitButton) {
        m_exitButton->setGeometry(kExitButtonX, kExitButtonY, kExitButtonWidth, kExitButtonHeight);
    }
}

void GameView::showSettingPanel() {
    if (!m_settingPanel) {
        return;
    }
    // 显示前按当前窗口尺寸重算一次设置子界面布局。
    resizeButtons();
    if (m_data) {
        setValueSilently(m_speedSlider, m_data->fallSpeed());
        if (m_speedValueLabel) {
            m_speedValueLabel->setText(QString::number(m_data->fallSpeed()));
        }
        setValueSilently(m_maxAppleSlider, m_data->maxAppleCount());
        if (m_maxAppleValueLabel) {
            m_maxAppleValueLabel->setText(QString::number(m_data->maxAppleCount()));
        }
        setValueSilently(m_targetSlider, m_data->targetCount());
        if (m_targetValueLabel) {
            m_targetValueLabel->setText(QString::number(m_data->targetCount()));
        }
    }
    m_settingPanel->setVisible(true);
    m_settingPanel->raise();
    m_settingPanel->activateWindow();
    emit settingPanelShown();
}

void GameView::hideSettingPanel() {
    if (!m_settingPanel) {
        return;
    }
    m_settingPanel->setVisible(false);
    emit settingPanelHidden();
}

void GameView::onModelChanged() {
    update();
}

QRectF GameView::basketRect() const {
    const int bw = m_basket.isNull() ? kFallbackBasketWidth : m_basket.width();
    const int bh = m_basket.isNull() ? kFallbackBasketHeight : m_basket.height();
    // 篮子固定在右下角，使用固定边距。
    const int x = width() - bw - kBasketRightMargin;
    const int y = height() - bh - kBasketBottomMargin;
    return QRectF(x, y, bw, bh);
}

QSize GameView::fruitSize(FruitType type) const {
    const QPixmap px = spriteFor(type);
    if (px.isNull()) {
        const int s = static_cast<int>(kFallbackFruitSize * kFruitSpriteScale);
        return QSize(s, s);
    }
    return QSize(static_cast<int>(px.width() * kFruitSpriteScale), static_cast<int>(px.height() * kFruitSpriteScale));
}

QPixmap GameView::spriteFor(FruitType type) const {
    if (type == FruitType::BadApple) {
        return m_badApple;
    }
    if (type == FruitType::SmallApple) {
        return m_smallApple;
    }
    return m_goodApple;
}

void GameView::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (!m_background.isNull()) {
        p.drawPixmap(rect(), m_background);
    } else {
        p.fillRect(rect(), QColor(230, 236, 248));
    }

    if (m_data) {
        for (const Fruit& fruit : m_data->fruits()) {
            const QPixmap sprite = spriteFor(fruit.type());
            if (sprite.isNull()) {
                p.setPen(Qt::NoPen);
                p.setBrush(fruit.type() == FruitType::BadApple ? Qt::red : Qt::green);
                p.drawEllipse(fruit.position(), 14, 14);
            } else {
                const QSize drawSize = fruitSize(fruit.type());
                const QPixmap scaledSprite =
                    sprite.scaled(drawSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                p.drawPixmap(fruit.position() - QPointF(scaledSprite.width() * 0.5, scaledSprite.height() * 0.5),
                             scaledSprite);
            }
            if (fruit.type() != FruitType::BadApple && fruit.smashFrames() <= 0) {
                QFont f = p.font();
                f.setFamily(kLatinFontFamily);
                f.setBold(true);
                f.setPointSize(16);
                p.setFont(f);
                p.setPen(Qt::white);
                const QRectF textRect(fruit.position().x() - 18, fruit.position().y() - 16, 36, 32);
                p.drawText(textRect, Qt::AlignCenter, QString(fruit.letter()));
            }
        }
    }

    const QRectF basket = basketRect();
    if (!m_basket.isNull()) {
        p.drawPixmap(basket.toRect(), m_basket);
    } else {
        p.setBrush(QColor(125, 83, 40));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(basket, 8, 8);
    }

    const QRectF hudRect(kHudX, kHudY, kHudW, kHudH);
    p.setPen(QPen(QColor(255, 255, 255, 180), 1));
    p.setBrush(QColor(20, 30, 45, 170));
    p.drawRoundedRect(hudRect, 10, 10);
    if (m_data) {
        QFont hudFont = p.font();
        hudFont.setFamily(kLatinFontFamily);
        hudFont.setPointSize(12);
        hudFont.setBold(true);
        p.setFont(hudFont);

        const QString labels[4] = {
            QStringLiteral("Score:"), QStringLiteral("Life:"), QStringLiteral("Speed:"), QStringLiteral("Target:")};
        const QString values[4] = {QString::number(m_data->score()),
                                   QString::number(m_data->lives()),
                                   QString::number(m_data->fallSpeed()),
                                   QString::number(m_data->targetCount())};

        const qreal rowH = hudRect.height() / 4.0;
        const qreal labelX = hudRect.x() + 22.0;
        const qreal labelGap = 12.0;
        QFontMetrics fm(hudFont);
        int maxLabelPx = 0;
        for (int i = 0; i < 4; ++i) {
            maxLabelPx = qMax(maxLabelPx, fm.horizontalAdvance(labels[i]));
        }
        const qreal labelW = static_cast<qreal>(maxLabelPx) + 4.0;
        const qreal valueX = labelX + labelW + labelGap;
        const qreal valueW = hudRect.width() - (valueX - hudRect.x()) - 22.0;
        p.setPen(Qt::white);
        for (int i = 0; i < 4; ++i) {
            const qreal y = hudRect.y() + rowH * i;
            const QRectF labelRect(labelX, y, labelW, rowH);
            const QRectF valueRect(valueX, y, valueW, rowH);
            p.drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter, labels[i]);
            p.drawText(valueRect, Qt::AlignRight | Qt::AlignVCenter, values[i]);
        }
    }

    if (m_data && m_data->isGameOver()) {
        p.setBrush(QColor(0, 0, 0, 150));
        p.setPen(Qt::NoPen);
        p.drawRect(rect());
        QFont overFont = p.font();
        overFont.setFamily(kLatinFontFamily);
        overFont.setPointSize(28);
        overFont.setBold(true);
        p.setFont(overFont);
        p.setPen(Qt::white);
        p.drawText(rect(), Qt::AlignCenter, "Game Over");
    } else if (m_data && m_data->isPassed()) {
        p.setBrush(QColor(0, 0, 0, 120));
        p.setPen(Qt::NoPen);
        p.drawRect(rect());
        QFont passFont = p.font();
        passFont.setFamily(kChineseFontFamily);
        passFont.setPointSize(kPassedTextPointSize);
        passFont.setBold(true);
        p.setFont(passFont);
        p.setPen(kPassedTextColor);
        p.drawText(rect(), Qt::AlignCenter, kPassedText);
    }
}

void GameView::keyPressEvent(QKeyEvent* event) {
    const QString text = event->text().trimmed();
    if (!text.isEmpty()) {
        const QChar c = text.at(0).toUpper();
        if (c >= QChar('A') && c <= QChar('Z')) {
            emit letterPressed(c);
        }
    }
    QWidget::keyPressEvent(event);
}
