/* -------------------------------------------------------------------------
//  文件名    : GameController.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : SaveApple 控制器——信号槽连接、水果生成与命中判定
// -------------------------------------------------------------------------*/

#pragma once

#include <QObject>
#include <QTimer>

class GameData;
class GameView;

class GameController : public QObject {
    Q_OBJECT

public:
    // 构造控制器：连接 View 信号，初始化定时器与状态。
    GameController(GameData* model, GameView* view, QObject* parent = nullptr);
    // 主动开始游戏（可被外部或按钮触发）。
    void startGame();

public:
    // 开始按钮点击处理：进入开始状态并启动定时器。
    void onStartButtonClicked();
    // 暂停按钮点击处理：切换暂停/继续。
    void onPauseButtonClicked();
    // 结束按钮点击处理：结束本局（game over 或重置逻辑）。
    void onEndButtonClicked();
    // 退出按钮点击处理：退出程序。
    void onExitButtonClicked();
    // 键盘字母输入处理：判定命中并更新模型。
    void onLetterPressed(QChar letter);
    // 设置项：速度变化处理（直接影响水果下落速度）。
    void onSpeedChanged(int speed);
    // 设置项：同屏最大数量变化处理（影响生成上限）。
    void onMaxAppleChanged(int count);
    // 设置项：目标数量变化处理（影响过关条件）。
    void onTargetChanged(int target);
    // 设置面板显示处理：通常用于自动暂停等。
    void onSettingPanelShown();
    // 设置面板隐藏处理：通常用于恢复暂停状态等。
    void onSettingPanelHidden();
    // 游戏主循环更新：移动水果、碰撞、失败判定等。
    void updateGame();
    // 生成一个新的水果（受 maxAppleCount 等限制）。
    void spawnFruit();

private:
    GameData* m_model;
    GameView* m_view;
    QTimer m_gameTimer;
    QTimer m_spawnTimer;
    bool m_paused;
    bool m_wasPausedBeforeSetting;
    bool m_started;
    bool m_pausedBySetting;

    void recordRuntimeEvent(const char* eventName);
};
