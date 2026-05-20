/* -------------------------------------------------------------------------
//  文件名    : gui_mode.cpp
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : 创建 Apple / Space 游戏窗口（模态、退出返回启动器）
// -------------------------------------------------------------------------*/

#include "gui_mode.h"

#include "Controller/GameController.h"
#include "Model/GameData.h"
#include "View/GameView.h"

#include "Controller/SpaceController.h"
#include "Model/SpaceModel.h"
#include "View/SpaceView.h"

#include "exam/GameWindow.h"

#include <QApplication>
#include <QScreen>

void runAppleGui() {
    auto* model = new GameData;
    auto* view  = new GameView(model);
    auto* ctrl  = new GameController(model, view);

    view->setWindowTitle("Save Apple - TypeGame");
    view->setWindowFlag(Qt::FramelessWindowHint, false);
    view->setWindowFlag(Qt::WindowMaximizeButtonHint, false);
    const QRect screenRect = QApplication::primaryScreen()->availableGeometry();
    view->setGeometry(screenRect.x() + screenRect.width() / 8,
                      screenRect.y() + screenRect.height() / 8,
                      screenRect.width() * 3 / 4, screenRect.height() * 3 / 4);
    view->setFixedSize(screenRect.width() * 3 / 4, screenRect.height() * 3 / 4);

    setupGameWindow(view, &GameView::exitClicked, [view, ctrl] {
        view->hide(); ctrl->onEndButtonClicked();
        view->deleteLater(); ctrl->deleteLater();
    });
    view->show();
}

void runSpaceGui() {
    auto* model = new SpaceModel;
    auto* view  = new SpaceView(model);
    auto* ctrl  = new SpaceController(model, view);

    view->setWindowTitle("SpaceBattle - TypeGame");
    const QSize sz = QApplication::primaryScreen()->availableSize();
    const int h = sz.height() * 3 / 4;
    view->setFixedSize(h * 4 / 3, h);

    setupGameWindow(view, &SpaceView::exitClicked, [view, ctrl] {
        view->hide(); ctrl->onReturnToMenu();
        view->deleteLater(); ctrl->deleteLater();
    });
    view->show();
}
