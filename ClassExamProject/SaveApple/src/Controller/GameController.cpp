#include "GameController.h"

#include "Config/GameConfig.h"
#include "Model/Fruit.h"
#include "Model/GameData.h"
#include "System/AnalyticsManager.h"
#include "System/AsyncLogger.h"
#include "System/AppleAudio.h"
#include "View/GameView.h"

#include <QApplication>
#include <QRandomGenerator>

GameController::GameController(GameData* model, GameView* view, QObject* parent)
    : QObject(parent),
      m_model(model),
      m_view(view),
      m_paused(false),
      m_wasPausedBeforeSetting(false),
      m_started(false),
      m_pausedBySetting(false) {
    AsyncLogger::instance().info("game controller created");
    connect(&m_gameTimer, &QTimer::timeout, this, &GameController::updateGame);
    connect(&m_spawnTimer, &QTimer::timeout, this, &GameController::spawnFruit);
    if (m_view) {
        connect(m_view, &GameView::letterPressed, this, &GameController::onLetterPressed);
        connect(m_view, &GameView::startClicked, this, &GameController::onStartButtonClicked);
        connect(m_view, &GameView::pauseClicked, this, &GameController::onPauseButtonClicked);
        connect(m_view, &GameView::endClicked, this, &GameController::onEndButtonClicked);
        connect(m_view, &GameView::exitClicked, this, &GameController::onExitButtonClicked);
        connect(m_view, &GameView::speedChanged, this, &GameController::onSpeedChanged);
        connect(m_view, &GameView::maxAppleChanged, this, &GameController::onMaxAppleChanged);
        connect(m_view, &GameView::targetChanged, this, &GameController::onTargetChanged);
        connect(m_view, &GameView::settingPanelShown, this, &GameController::onSettingPanelShown);
        connect(m_view, &GameView::settingPanelHidden, this, &GameController::onSettingPanelHidden);
    }
}

void GameController::startGame() {
    if (!m_model) {
        return;
    }
    m_model->reset();
    AnalyticsManager::instance().startSession();
    recordRuntimeEvent("game_start");
    AppleAudio::instance().startBgm();
    m_started = true;
    m_paused = false;
    m_pausedBySetting = false;
    const GameConfig& cfg = GameConfig::instance();
    m_gameTimer.start(cfg.updateIntervalMs());
    m_spawnTimer.start(cfg.spawnIntervalMs());
    if (m_view) {
        m_view->setMainControlState(true);
    }
}

void GameController::onStartButtonClicked() {
    if (m_started) {
        return;
    }
    startGame();
}

void GameController::onPauseButtonClicked() {
    if (!m_started || (m_model && m_model->isGameOver())) {
        return;
    }
    m_paused = !m_paused;
    if (m_paused) {
        recordRuntimeEvent("game_paused");
        AppleAudio::instance().pauseBgm();
        m_gameTimer.stop();
        m_spawnTimer.stop();
        return;
    }
    recordRuntimeEvent("game_resumed");
    AppleAudio::instance().resumeBgm();
    const GameConfig& cfg = GameConfig::instance();
    m_gameTimer.start(cfg.updateIntervalMs());
    m_spawnTimer.start(cfg.spawnIntervalMs());
}

void GameController::onEndButtonClicked() {
    recordRuntimeEvent("game_end");
    AppleAudio::instance().stopBgm();
    m_started = false;
    m_paused = false;
    m_pausedBySetting = false;
    m_gameTimer.stop();
    m_spawnTimer.stop();
    if (m_model) {
        AnalyticsManager::instance().finishSession(m_model->score(), m_model->isPassed());
        m_model->reset();
    }
    if (m_view) {
        m_view->setMainControlState(false);
    }
}

void GameController::onExitButtonClicked() {
    recordRuntimeEvent("game_exit");
    onEndButtonClicked();
    QApplication::quit();
}

void GameController::onLetterPressed(QChar letter) {
    if (!m_model || m_model->isGameOver()) {
        return;
    }
    QVector<Fruit>& fruits = m_model->editableFruits();
    int hitIndex = -1;
    qreal maxY = -1;
    for (int i = 0; i < fruits.size(); ++i) {
        const Fruit& f = fruits.at(i);
        if (f.type() == FruitType::BadApple || f.smashFrames() > 0) {
            continue;
        }
        if (f.letter() == letter && f.position().y() > maxY) {
            maxY = f.position().y();
            hitIndex = i;
        }
    }
    if (hitIndex >= 0) {
        AnalyticsManager::instance().recordEvent("hit");
        recordRuntimeEvent("hit_letter");
        AppleAudio::instance().play(AppleAudioEvent::AppleIn);
        m_model->addScore(fruits.at(hitIndex).scoreValue());
        m_model->removeFruitAt(hitIndex);
        m_model->recordHit();
        if (m_model->isPassed()) {
            AnalyticsManager::instance().recordEvent("pass");
            recordRuntimeEvent("game_passed");
            AppleAudio::instance().stopBgm();
            // 过关后结束当前局面：停止刷帧/刷怪并清空场景，等待用户再次点击“开始”。
            m_started = false;
            m_paused = false;
            m_pausedBySetting = false;
            m_wasPausedBeforeSetting = false;
            m_gameTimer.stop();
            m_spawnTimer.stop();
            m_model->editableFruits().clear();
            if (m_view) {
                m_view->setMainControlState(false);
            }
        }
        m_model->notifyObservers();
        return;
    }
    AnalyticsManager::instance().recordEvent("miss_key");
    recordRuntimeEvent("miss_letter");
    m_model->notifyObservers();
}

void GameController::onMaxAppleChanged(int count) {
    if (!m_model) {
        return;
    }
    m_model->setMaxAppleCount(count);
    m_model->notifyObservers();
}

void GameController::onSpeedChanged(int speed) {
    if (!m_model) {
        return;
    }
    m_model->setFallSpeed(speed);
    m_model->notifyObservers();
}

void GameController::onTargetChanged(int target) {
    if (!m_model) {
        return;
    }
    m_model->setTargetCount(target);
    m_model->notifyObservers();
}

void GameController::onSettingPanelShown() {
    if (!m_model || !m_started || m_model->isGameOver()) {
        return;
    }
    m_wasPausedBeforeSetting = m_paused;
    m_pausedBySetting = false;
    if (!m_paused) {
        m_paused = true;
        m_gameTimer.stop();
        m_spawnTimer.stop();
        m_pausedBySetting = true;
    }
}

void GameController::onSettingPanelHidden() {
    if (!m_model || !m_started || m_model->isGameOver()) {
        return;
    }
    if (m_pausedBySetting && !m_wasPausedBeforeSetting) {
        const GameConfig& cfg = GameConfig::instance();
        m_paused = false;
        m_gameTimer.start(cfg.updateIntervalMs());
        m_spawnTimer.start(cfg.spawnIntervalMs());
    }
    m_pausedBySetting = false;
}

void GameController::spawnFruit() {
    if (!m_model || !m_view || m_model->isGameOver()) {
        return;
    }
    if (m_model->fruits().size() >= m_model->maxAppleCount()) {
        return;
    }
    const int roll = QRandomGenerator::global()->bounded(100);
    const FruitType type = (roll < 78) ? FruitType::GoodApple : FruitType::SmallApple;
    const int spawnX = QRandomGenerator::global()->bounded(qMax(40, m_view->width() - 40));
    const qreal speed = static_cast<qreal>(m_model->fallSpeed());
    const QChar pickedLetter(static_cast<char>('A' + QRandomGenerator::global()->bounded(26)));

    Fruit fruit(type, QPointF(spawnX, -30), speed);
    fruit.setLetter(pickedLetter);
    m_model->addFruit(fruit);
    m_model->notifyObservers();
}

void GameController::updateGame() {
    if (!m_model || !m_view || m_model->isGameOver()) {
        return;
    }

    QVector<Fruit> next;
    next.reserve(m_model->fruits().size());

    const int prevLives = m_model->lives();
    for (Fruit fruit : m_model->fruits()) {
        if (fruit.smashFrames() > 0) {
            fruit.tickSmashFrame();
            if (fruit.smashFrames() > 0) {
                next.push_back(fruit);
            }
            continue;
        }
        fruit.update(1.0f);
        const QRectF bounds = fruit.bounds(m_view->fruitSize(fruit.type()));

        if (bounds.top() > m_view->height() * 0.70) {
            fruit.setType(FruitType::BadApple);
            fruit.setSpeed(0);
            fruit.setSmashFrames(30);
            const int badH = m_view->fruitSize(FruitType::BadApple).height();
            const qreal failLineY = m_view->height() * 0.70;
            fruit.setPosition(QPointF(fruit.position().x(), failLineY + badH * 0.5));
            m_model->loseLife();
            AnalyticsManager::instance().recordEvent("lose_life");
            recordRuntimeEvent("life_lost");
            if (!m_model->isGameOver()) {
                next.push_back(fruit);
            }
            continue;
        }
        next.push_back(fruit);
    }

    m_model->editableFruits() = next;
    if (m_model->isGameOver()) {
        if (m_model->lives() < prevLives) {
            AnalyticsManager::instance().recordEvent("last_life_lost");
        }
        AnalyticsManager::instance().recordEvent("game_over");
        recordRuntimeEvent("game_over");
        AppleAudio::instance().stopBgm();
        m_started = false;
        m_spawnTimer.stop();
        m_gameTimer.stop();
        if (m_view) {
            m_view->setMainControlState(false);
        }
    }
    m_model->notifyObservers();
}

void GameController::recordRuntimeEvent(const char* eventName) {
    AnalyticsManager::instance().recordEvent(eventName);
    AsyncLogger::instance().info(std::string("event=") + eventName);
}
