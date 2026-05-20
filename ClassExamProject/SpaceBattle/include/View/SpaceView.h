#pragma once

/// @file SpaceView.h
/// 太空大战 · 视图（View）：Qt Widget + QGraphicsView 混合 UI（菜单/设置/高分叠层 + 战斗场景绘制）。
/// 仅负责展示与用户输入，通过 signal 通知 Controller；不直接修改游戏规则。

#include "Model/SpaceModel.h"
#include "View/EnemyCaptionLetterFactory.h"

#include <QPixmap>
#include <QSize>
#include <QVector>
#include <QWidget>

class ButtonIconManager;

class QLabel;
class QPushButton;
class QSlider;
class QGraphicsView;
class QGraphicsScene;
class QGraphicsItemGroup;
class QGraphicsPixmapItem;
class QGraphicsTextItem;

class SpaceView : public QWidget {
    Q_OBJECT
public:
    explicit SpaceView(SpaceModel* model, QWidget* parent = nullptr);

signals:
    void startClicked();
    void pauseClicked();
    void exitClicked();
    void settingsApplied(const SpaceSettings& s);
    void letterPressed(QChar letter);
    void rewardCharTyped(QChar c);
    void returnToMenu();
    void dismissGameOver();

public slots:
    void showSettings();
    void hideSettings();
    void showHiscorePanel();
    void hideHiscorePanel();
    /// 根据模型状态切换菜单/暂停/游戏结束可见性，并在战斗中刷新动态场景。
    /// @param gameTickDeltaSec 本帧物理步长（秒），与 SpaceController::tick 中 dt 一致；负值表示非 tick 调用（如回菜单）。
    void refreshUi(double gameTickDeltaSec = -1.0);
    void saveScore(int score);

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void resizeEvent(QResizeEvent*) override;
    /// 非战斗画面（菜单背景等）由 QWidget 自绘。
    void paintEvent(QPaintEvent*) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void loadAssets();
    void loadScores();
    void setupUi();
    void setupMenuUi();
    void setupSettingsPanel();
    void setupHiscorePanel();
    /// 构建星空/HUD 等不变图层（Playing 时首次进入战斗调用）。
    void createStaticScene();
    /// 每帧重绘飞船、敌人、子弹与奖励词文本。
    void updateDynamicScene(double gameTickDeltaSec);
    void layoutAll();
    /// 参考窗 1704×1278 的比例工具（layout_export 坐标系）。
    void layoutGameView(int w, int h);
    void layoutMainMenu(int w, int h);
    void layoutBattleOverlays(int w, int h);
    void layoutSettingsPanel(int w, int h);
    void layoutHiscorePanel(int w, int h);
    void layoutStarfieldCache(int w, int h);
    void paintStarfieldFromParticles();

    SpaceModel* m_model = nullptr;

    // ========== assets ==========
    QPixmap m_bg;
    QPixmap m_menuBg;
    QPixmap m_ship;
    QPixmap m_enemyA;
    QPixmap m_enemyB;
    QPixmap m_meteorFull;
    QPixmap m_explosionFull;
    QPixmap m_captionBack;
    QPixmap m_bomb;
    QPixmap m_labelLife;
    QPixmap m_labelScore;
    QPixmap m_labelTime;
    QPixmap m_lifeBar;
    QPixmap m_lifeOver;
    /// SPACE_STARS：5 种 1×1 星（条带切割见 cpp）；每帧写入与窗口同大的位图，夹在背景与 HUD 之间。
    QPixmap m_stars;
    QPixmap m_starfieldCache;
    QVector<QPixmap> m_starPix1x1;
    struct StarParticle {
        float x = 0, y = 0, vx = 0, vy = 0;
        quint8 kind = 0;
    };
    QVector<StarParticle> m_starParticles;
    QSize m_starfieldLayoutSize;

    // ========== menu UI (normal widgets) ==========
    QPushButton* m_btnStart = nullptr;
    QPushButton* m_btnHiscore = nullptr;
    QPushButton* m_btnOption = nullptr;
    QPushButton* m_btnExit = nullptr;
    QWidget* m_gameOverOverlay = nullptr;
    QLabel* m_gameOverLabel = nullptr;
    QLabel* m_pausedLabel = nullptr;

    // ========== settings panel (normal widget overlay) ==========
    QWidget* m_settingsPanel = nullptr;
    QLabel* m_settingsBgLabel = nullptr;
    QLabel* m_labelEnemyCount = nullptr;
    QLabel* m_labelEnemyValue = nullptr;
    QSlider* m_sliderEnemyCount = nullptr;
    QLabel* m_labelSpeed = nullptr;
    QLabel* m_labelSpeedValue = nullptr;
    QSlider* m_sliderSpeed = nullptr;
    QLabel* m_labelUpgrade = nullptr;
    QLabel* m_labelUpgradeValue = nullptr;
    QSlider* m_sliderUpgrade = nullptr;
    QPushButton* m_checkReward = nullptr;
    QPixmap m_checkUnchecked;
    QPixmap m_checkUncheckedHover;
    QPixmap m_checkChecked;
    QPixmap m_checkCheckedHover;
    QVector<int> m_highScores;

    QPushButton* m_btnSettingsOk = nullptr;
    QPushButton* m_btnSettingsCancel = nullptr;
    QPushButton* m_btnSettingsDefault = nullptr;

    // ========== hiscore panel (overlay popup) ==========
    QWidget* m_hiscorePanel = nullptr;
    QLabel* m_hiscoreBgLabel = nullptr;
    QLabel* m_hiscoreScores[9] = {};
    QPushButton* m_btnHiscoreBack = nullptr;

    // ========== game view (QGraphicsView) ==========
    QGraphicsView* m_gameView = nullptr;
    QGraphicsScene* m_gameScene = nullptr;
    QGraphicsItemGroup* m_staticGroup = nullptr;
    QGraphicsItemGroup* m_dynamicGroup = nullptr;

    QGraphicsPixmapItem* m_sceneLifeFill = nullptr;
    QGraphicsPixmapItem* m_sceneStarsItem = nullptr;
    QGraphicsTextItem* m_sceneScoreText = nullptr;
    QGraphicsTextItem* m_sceneTimeText = nullptr;
    bool m_staticSceneCreated = false;

    /// 敌机 / 陨石共用一套字幕与载体尺寸；陨石仅字幕相对载体的垂直偏移沿用 MeteorClusterStrategy。
    ClusterLayoutProfile m_enemyClusterTemplate;
    int m_meteorCaptionFromCarrierY = 0;

    ButtonIconManager* m_btnIcons = nullptr;
};
