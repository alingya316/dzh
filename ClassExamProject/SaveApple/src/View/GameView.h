/* -------------------------------------------------------------------------
//  文件名    : GameView.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : SaveApple 视图——QPainter 绘制背景、水果、篮子、HUD
// -------------------------------------------------------------------------*/

#pragma once

#include <exam/IObserver.h>

#include "Model/Fruit.h"

#include <QPixmap>
#include <QPushButton>
#include <QSize>
#include <QWidget>

class ButtonIconManager;

class GameData;
class QLabel;
class QSlider;

class GameView : public QWidget, public IObserver {
    Q_OBJECT

public:
    // 构造视图：绑定模型，初始化资源和界面。
    explicit GameView(GameData* data, QWidget* parent = nullptr);
    // 析构视图：取消观察者绑定，避免模型回调到已销毁对象。
    ~GameView() override;

    // 模型通知回调：数据变化后触发界面刷新。
    void onModelChanged() override;

    // 返回篮子碰撞区域（供控制器做命中判定）。
    QRectF basketRect() const;
    // 返回指定水果类型的绘制尺寸（无资源时返回兜底尺寸）。
    QSize fruitSize(FruitType type) const;
    // 根据游戏是否进行中，更新主功能按钮可用状态。
    void setMainControlState(bool running);

signals:
    // 键盘字母输入信号（A-Z），由控制器处理命中逻辑。
    void letterPressed(QChar letter);
    // 开始按钮点击信号。
    void startClicked();
    // 暂停按钮点击信号。
    void pauseClicked();
    // 结束按钮点击信号。
    void endClicked();
    // 退出按钮点击信号。
    void exitClicked();
    // 设置项：速度变化信号。
    void speedChanged(int speed);
    // 设置项：同屏最大苹果数量变化信号。
    void maxAppleChanged(int count);
    // 设置项：过关目标变化信号。
    void targetChanged(int target);
    // 设置面板显示信号（控制器可据此暂停游戏）。
    void settingPanelShown();
    // 设置面板隐藏信号（控制器可据此恢复游戏）。
    void settingPanelHidden();

protected:
    // Qt 绘制回调：绘制背景、苹果、篮子、HUD 等内容。
    void paintEvent(QPaintEvent* event) override;
    // Qt 键盘回调：捕获按键并发出字母信号。
    void keyPressEvent(QKeyEvent* event) override;

private:
    // 加载游戏资源图片（背景、苹果、篮子等）。
    void loadAssets();
    // 按水果类型选择对应贴图。
    QPixmap spriteFor(FruitType type) const;
    // 创建 UI 控件并连接信号。
    void setupUi();
    // 主窗口 UI（底部按钮等）。
    void setupMainUi();
    // 设置子窗口 UI（面板、滑条、按钮等）。
    void setupSettingUi();
    // 统一设置字体样式。
    void applyLatinValueFont(QLabel* label);
    // 按当前窗口大小重新计算按钮与面板布局。
    void resizeButtons();
    // 应用默认主功能按钮布局（使用常量参数）。
    void applyDefaultMainButtonLayout();
    // 显示设置面板并同步模型数据到控件。
    void showSettingPanel();
    // 隐藏设置面板。
    void hideSettingPanel();

    GameData* m_data;
    // 背景与精灵贴图资源（由 GameConfig 提供路径后加载）。
    QPixmap m_background;
    QPixmap m_goodApple;
    QPixmap m_smallApple;
    QPixmap m_badApple;
    QPixmap m_basket;
    // 主界面按钮（开始/暂停/结束/退出/设置）。
    QPushButton* m_startButton;
    QPushButton* m_pauseButton;
    QPushButton* m_endButton;
    QPushButton* m_exitButton;
    QPushButton* m_setupButton;

    // 设置子窗口及其背景图。
    QWidget* m_settingPanel;
    QLabel* m_settingBgLabel;
    // 设置子窗口底部按钮（取消/确定/默认）。
    QPushButton* m_settingCancelButton;
    QPushButton* m_settingOkButton;
    QPushButton* m_settingDefaultButton;
    // 设置子窗口控件：下落速度。
    QLabel* m_speedLabel;
    QLabel* m_speedValueLabel;
    QSlider* m_speedSlider;
    // 设置子窗口控件：同屏最大数量。
    QLabel* m_maxAppleLabel;
    QLabel* m_maxAppleValueLabel;
    QSlider* m_maxAppleSlider;
    // 设置子窗口控件：目标数量。
    QLabel* m_targetLabel;
    QLabel* m_targetValueLabel;
    QSlider* m_targetSlider;
    ButtonIconManager* m_btnIcons = nullptr;
};
