#include "GameData.h"

#include <algorithm>
#include <QtGlobal>

GameData::GameData()
    : m_score(0),
      m_lives(3),
      m_gameOver(false),
      m_passed(false),
      m_successCount(0),
      m_fallSpeed(4),
      m_maxAppleCount(3),
      m_targetCount(15) {}

int GameData::score() const {
    return m_score;
}

int GameData::lives() const {
    return m_lives;
}

bool GameData::isGameOver() const {
    return m_gameOver;
}

bool GameData::isPassed() const {
    return m_passed;
}

int GameData::fallSpeed() const {
    return m_fallSpeed;
}

int GameData::maxAppleCount() const {
    return m_maxAppleCount;
}

int GameData::targetCount() const {
    return m_targetCount;
}

const QVector<Fruit>& GameData::fruits() const {
    return m_fruits;
}

QVector<Fruit>& GameData::editableFruits() {
    return m_fruits;
}

void GameData::reset() {
    m_score = 0;
    m_lives = 3;
    m_gameOver = false;
    m_passed = false;
    m_successCount = 0;
    m_fruits.clear();
    notifyObservers();
}

void GameData::addScore(int value) {
    m_score += value;
}

void GameData::loseLife() {
    if (m_lives > 0) {
        --m_lives;
    }
    if (m_lives <= 0) {
        m_lives = 0;
        m_gameOver = true;
    }
}

void GameData::addFruit(const Fruit& fruit) {
    m_fruits.push_back(fruit);
}

void GameData::removeFruitAt(int index) {
    if (index >= 0 && index < m_fruits.size()) {
        m_fruits.removeAt(index);
    }
}

void GameData::recordHit() {
    ++m_successCount;
    checkPassProgress();
}

void GameData::checkPassProgress() {
    if (m_successCount >= m_targetCount) {
        m_successCount = 0;
        m_passed = true;
    }
}

void GameData::setMaxAppleCount(int count) {
    m_maxAppleCount = qBound(1, count, 5);
}

void GameData::setFallSpeed(int speed) {
    m_fallSpeed = qBound(1, speed, 10);
}

void GameData::setTargetCount(int count) {
    m_targetCount = qBound(5, count, 100);
}

void GameData::attach(IObserver* observer) {
    if (!observer) {
        return;
    }
    if (!m_observers.contains(observer)) {
        m_observers.push_back(observer);
    }
}

void GameData::detach(IObserver* observer) {
    auto it = std::remove(m_observers.begin(), m_observers.end(), observer);
    m_observers.erase(it, m_observers.end());
}

void GameData::notifyObservers() {
    for (IObserver* observer : m_observers) {
        if (observer) {
            observer->onModelChanged();
        }
    }
}
