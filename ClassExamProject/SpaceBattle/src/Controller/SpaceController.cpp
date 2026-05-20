/// @file SpaceController.cpp
/// 游戏规则实现：定时生成敌人、键盘制导子弹、碰撞与得分、奖励模式与难度曲线。

#include "Controller/SpaceController.h"

#include "System/SpaceAudio.h"
#include "System/WordProvider.h"
#include "View/SpaceView.h"

#include <QDateTime>
#include <QApplication>
#include <QRandomGenerator>
#include <QtMath>

#include <algorithm>

namespace {
constexpr int kTickMs = 16;           ///< 主循环间隔（约 60 FPS）
constexpr int kSpawnMsBase = 900;     ///< 初始敌人生成间隔（毫秒），随难度缩短
constexpr qreal kEnemySize = 128.0; ///< 敌人/陨石包围盒边长（与绘制一致）
constexpr qreal kShipSize = 160.0;
constexpr qreal kBulletSpeed = 520.0;
constexpr qreal kBulletTurnRate = 8.0; ///< 追踪弹转向强度（越大拐弯越快）
constexpr qreal kEnemyBaseSpeed = 80.0;
constexpr qreal kEnemySineAmp = 60.0;   ///< 飞机左右摆动幅度
constexpr qreal kEnemySineFreq = 1.3;
constexpr qreal kHitRadius = 52.0;
constexpr qreal kShipSpeed = 280.0;
constexpr qreal kShipCollisionHalf = 50.0;
}

// ---------- 构造与槽函数 ----------

SpaceController::SpaceController(SpaceModel* model, SpaceView* view, QObject* parent)
    : QObject(parent)
    , m_model(model)
    , m_view(view)
    , m_wordProvider(new WordProvider(this)) {
    if (m_model) {
        m_model->reset();
        m_model->setShipPos(QPointF(400, 520));
    }

    connect(&m_tickTimer, &QTimer::timeout, this, &SpaceController::tick);
    connect(&m_spawnTimer, &QTimer::timeout, this, &SpaceController::spawnEnemy);
    connect(&m_upgradeTimer, &QTimer::timeout, this, &SpaceController::upgradeDifficulty);
    connect(&m_rewardTimer, &QTimer::timeout, this, &SpaceController::spawnRewardWord);

    if (m_view) {
        connect(m_view, &SpaceView::startClicked, this, &SpaceController::onStart);
        connect(m_view, &SpaceView::pauseClicked, this, &SpaceController::onPause);
        connect(m_view, &SpaceView::exitClicked, this, &SpaceController::onExit);
        connect(m_view, &SpaceView::returnToMenu, this, &SpaceController::onReturnToMenu);
        connect(m_view, &SpaceView::dismissGameOver, this, &SpaceController::onDismissGameOver);
        connect(m_view, &SpaceView::settingsApplied, this, &SpaceController::onSettingsApplied);
        connect(m_view, &SpaceView::letterPressed, this, &SpaceController::onLetterPressed);
        connect(m_view, &SpaceView::rewardCharTyped, this, &SpaceController::onRewardTyped);
    }

    connect(m_wordProvider, &WordProvider::wordReady, this, &SpaceController::onWordReady);

    m_tickTimer.start(kTickMs);
    m_spawnTimer.start(kSpawnMsBase);
}

int SpaceController::nextEnemyId() {
    return m_nextEnemyId++;
}

QChar SpaceController::nextUniqueLetter() const {
    bool used[26] = {};
    if (m_model) {
        for (const auto& e : m_model->enemies()) {
            if (e.alive && !e.exploding) {
                const int idx = e.letter.toUpper().unicode() - 'A';
                if (idx >= 0 && idx < 26) used[idx] = true;
            }
        }
    }
    QVector<int> candidates;
    for (int i = 0; i < 26; ++i) {
        if (!used[i]) candidates.push_back(i);
    }
    if (candidates.isEmpty()) {
        return QChar();
    }
    const int pick = candidates.at(QRandomGenerator::global()->bounded(candidates.size()));
    return QChar('A' + pick);
}

void SpaceController::onStart() {
    if (!m_model) return;
    m_baseSettings = m_model->settings(); // save user config before reset
    resetGame();
    m_model->setState(SpaceGameState::Playing);
    m_spawnTimer.start(kSpawnMsBase);
    SpaceAudio::instance().startBgm();
    m_lastMs = QDateTime::currentMSecsSinceEpoch();
    m_upgradeTimer.start(m_model->settings().upgradeIntervalSec * 1000);
    if (m_model->settings().rewardMode) {
        m_rewardTimer.start(10 * 1000);
    }
}

void SpaceController::onPause() {
    if (!m_model) return;
    if (m_model->state() == SpaceGameState::Playing) {
        m_model->setState(SpaceGameState::Paused);
        SpaceAudio::instance().pauseBgm();
        return;
    }
    if (m_model->state() == SpaceGameState::Paused) {
        m_model->setState(SpaceGameState::Playing);
        m_lastMs = QDateTime::currentMSecsSinceEpoch();
        SpaceAudio::instance().resumeBgm();
    }
}

void SpaceController::onExit() {
    SpaceAudio::instance().stopBgm();
    QApplication::quit();
}

void SpaceController::onReturnToMenu() {
    if (!m_model) return;
    const SpaceGameState st = m_model->state();
    if (st != SpaceGameState::Playing && st != SpaceGameState::Paused && st != SpaceGameState::GameOver)
        return;
    SpaceAudio::instance().stopBgm();
    m_spawnTimer.stop();
    m_upgradeTimer.stop();
    m_rewardTimer.stop();
    resetGame();
    m_model->setState(SpaceGameState::Initial);
    if (m_view) m_view->refreshUi();
}

void SpaceController::onDismissGameOver() {
    if (!m_model || m_model->state() != SpaceGameState::GameOver) return;
    onReturnToMenu();
}

void SpaceController::onSettingsApplied(const SpaceSettings& s) {
    if (!m_model) return;
    m_model->settings() = s;
    // timers update
    m_upgradeTimer.stop();
    if (m_model->state() == SpaceGameState::Playing) {
        m_upgradeTimer.start(m_model->settings().upgradeIntervalSec * 1000);
    }
    m_rewardTimer.stop();
    if (m_model->state() == SpaceGameState::Playing && m_model->settings().rewardMode) {
        m_rewardTimer.start(10 * 1000);
    }
}

// ---------- 内部逻辑 ----------

void SpaceController::resetGame() {
    if (!m_model) return;
    m_model->setShipAnimFrame(0);
    m_model->setLife(18);
    m_model->addScore(-m_model->score());
    m_model->enemies().clear();
    m_model->bullets().clear();
    m_model->typedRewardBuffer().clear();
    m_model->rewardWord() = RewardWord{};
    m_model->settings() = m_baseSettings;
    m_model->addElapsed(-m_model->elapsedSec());
    m_lastMs = 0;
    m_nextEnemyId = 1;
}

/// 在屏幕顶部随机 X 生成陨石或飞机，字母在当前场上未占用集合中随机。
void SpaceController::spawnEnemy() {
    if (!m_model || !m_view) return;
    if (m_model->state() != SpaceGameState::Playing) return;

    const QVector<SpaceEnemy>& aliveList = m_model->enemies();
    const int aliveCount = static_cast<int>(std::count_if(aliveList.cbegin(), aliveList.cend(),
        [](const SpaceEnemy& e) { return e.alive && !e.exploding; }));
    if (aliveCount >= m_model->settings().maxEnemies) {
        return;
    }

    const int w = m_view->width();
    const qreal x = QRandomGenerator::global()->bounded(qMax(1, w - static_cast<int>(kEnemySize)));
    SpaceEnemy e;
    e.id = nextEnemyId();
    e.type = (QRandomGenerator::global()->bounded(4) == 0) ? EnemyType::Meteor : EnemyType::Aircraft;
    e.pos = QPointF(x, -kEnemySize - 10);
    const qreal speedScale = 0.6 + (m_model->settings().speedLevel / 10.0);
    e.vel = QPointF(0, kEnemyBaseSpeed * speedScale);
    e.letter = nextUniqueLetter();
    if (e.letter.isNull()) {
        return;
    }
    m_model->enemies().push_back(e);

    SpaceAudio::instance().play(SpaceAudioEvent::PlaneOut);
}

/// 相同字母优先击打纵向位置最靠下的敌人；同一目标仅允许一发在途追踪弹。
void SpaceController::onLetterPressed(QChar c) {
    if (!m_model) return;
    if (m_model->state() != SpaceGameState::Playing) return;

    int targetId = -1;
    qreal bestY = -1e9;
    QPointF targetPos;
    for (const auto& e : m_model->enemies()) {
        if (!e.alive || e.exploding) continue;
        if (e.letter.toUpper() == c.toUpper()) {
            if (e.pos.y() > bestY) {
                bestY = e.pos.y();
                targetId = e.id;
                targetPos = e.pos;
            }
        }
    }
    if (targetId < 0) return;

    // 已有一颗子弹在追踪该敌机，不再重复发射
    const QVector<SpaceBullet>& bl = m_model->bullets();
    if (std::any_of(bl.cbegin(), bl.cend(), [targetId](const SpaceBullet& b) {
            return b.alive && b.targetEnemyId == targetId;
        }))
        return;

    SpaceBullet b;
    b.pos = m_model->shipPos() + QPointF(kShipSize * 0.5, 0);
    const QPointF dir = (targetPos - b.pos);
    const qreal len = qSqrt(dir.x() * dir.x() + dir.y() * dir.y());
    QPointF nd = (len > 1e-3) ? QPointF(dir.x() / len, dir.y() / len) : QPointF(0, -1);
    b.vel = nd * kBulletSpeed;
    b.targetEnemyId = targetId;
    m_model->bullets().push_back(b);
    SpaceAudio::instance().play(SpaceAudioEvent::Shoot);
}

/// 统一时间步：Playing 时更新实体并剔除死亡对象；生命归零进入 End 并写高分。
void SpaceController::tick() {
    if (!m_model || !m_view) return;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    double dt = 0.016;
    if (m_lastMs != 0) {
        dt = qBound(0.001, (now - m_lastMs) / 1000.0, 0.05);
    }
    m_lastMs = now;

    if (m_model->state() != SpaceGameState::Playing) {
        m_view->refreshUi(dt);
        return;
    }

    m_model->addElapsed(dt);
    updateShip(dt);
    updateEnemies(dt);
    updateBullets(dt);
    updateReward(dt);

    // Remove dead entities
    auto& enemies = m_model->enemies();
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [](const SpaceEnemy& e) { return !e.alive; }), enemies.end());
    auto& bullets = m_model->bullets();
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](const SpaceBullet& b) { return !b.alive; }), bullets.end());

    if (m_model->life() <= 0) {
        m_model->setState(SpaceGameState::GameOver);
        SpaceAudio::instance().stopBgm();
        m_spawnTimer.stop();
        m_upgradeTimer.stop();
        m_rewardTimer.stop();
        if (m_view) m_view->saveScore(m_model->score());
    }

    m_view->refreshUi(dt);
}

/// 飞船贴底横向往返移动，位置限制在窗口内。
void SpaceController::updateShip(double dt) {
    QPointF p = m_model->shipPos();
    QPointF v = m_model->shipVel();
    if (qFuzzyIsNull(v.x())) v.setX(kShipSpeed);
    p += v * dt;
    const qreal left = 0;
    const qreal right = m_view->width() - kShipSize;
    if (p.x() < left)      { p.setX(left);  v.setX(qAbs(v.x())); }
    else if (p.x() > right){ p.setX(right); v.setX(-qAbs(v.x())); }
    p.setY(m_view->height() - kShipSize - 10);
    m_model->setShipPos(p);
    m_model->setShipVel(v);
    m_model->advanceShipAnimFrame();
}

/// 下落、飞机正弦横移、敌机与陨石触底均扣血；陨石不与敌机相撞，仅可与飞船碰撞爆炸。
void SpaceController::updateEnemies(double dt) {
    const qreal h = m_view->height();
    const double t = m_model->elapsedSec();
    const QPointF ship = m_model->shipPos();
    for (auto& e : m_model->enemies()) {
        if (!e.alive) continue;
        if (e.exploding) {
            e.explosionFrame++;
            if (e.explosionFrame > 9) {
                e.alive = false;
            }
            continue;
        }
        // 3×4 精灵图：前 11 格循环（飞船与敌机共用同一套索引语义）
        e.animFrame = (e.animFrame + 1) % 11;

        // Movement
        e.pos.ry() += e.vel.y() * dt;
        if (e.type == EnemyType::Aircraft) {
            e.pos.rx() += qSin(t * kEnemySineFreq + e.id) * kEnemySineAmp * dt;
        }
        // Meteor: X doesn't change (straight down)
        e.pos.setX(qBound(0.0, e.pos.x(), m_view->width() - kEnemySize));

        // Bottom — 敌机与陨石漏到底部均扣生命（本帧不再判飞船重叠，避免双扣）
        if (e.pos.y() > h - kEnemySize) {
            e.alive = false;
            m_model->setLife(m_model->life() - 1);
            SpaceAudio::instance().play(SpaceAudioEvent::PlaneOut);
            continue;
        }

        // Ship collision — 陨石与敌机均可撞击飞船并爆炸扣血
        if (qAbs(e.pos.x() - ship.x()) < kShipCollisionHalf * 2 && qAbs(e.pos.y() - ship.y()) < kShipCollisionHalf * 2) {
            e.exploding = true;
            e.explosionFrame = 0;
            m_model->setLife(m_model->life() - 1);
            SpaceAudio::instance().play(SpaceAudioEvent::Blast);
        }
    }
}

/// 追踪目标中心，保持恒定速率；仅对 targetEnemyId 做命中判定。
void SpaceController::updateBullets(double dt) {
    for (auto& b : m_model->bullets()) {
        if (!b.alive) continue;

        QPointF targetPos;
        bool hasTarget = false;
        const QVector<SpaceEnemy>& enVec = m_model->enemies();
        const auto itEnemy = std::find_if(enVec.cbegin(), enVec.cend(), [&b](const SpaceEnemy& e) {
            return e.alive && !e.exploding && e.id == b.targetEnemyId;
        });
        if (itEnemy != enVec.cend()) {
            targetPos = itEnemy->pos + QPointF(kEnemySize * 0.5, kEnemySize * 0.5);
            hasTarget = true;
        }
        if (hasTarget) {
            QPointF desired = targetPos - b.pos;
            const qreal len = qSqrt(desired.x() * desired.x() + desired.y() * desired.y());
            if (len > 1e-3) {
                desired /= len;
                const QPointF cur = b.vel / qMax(1e-3, qSqrt(b.vel.x() * b.vel.x() + b.vel.y() * b.vel.y()));
                QPointF steer = desired - cur;
                b.vel += steer * (kBulletTurnRate * dt * kBulletSpeed);
            }
        }
        // normalize speed
        const qreal vlen = qSqrt(b.vel.x() * b.vel.x() + b.vel.y() * b.vel.y());
        if (vlen > 1e-3) {
            b.vel = b.vel * (kBulletSpeed / vlen);
        }
        b.pos += b.vel * dt;

        // out
        if (b.pos.x() < -20 || b.pos.x() > m_view->width() + 20 || b.pos.y() < -20 || b.pos.y() > m_view->height() + 20) {
            b.alive = false;
            continue;
        }

        // hit test — only against target enemy
        for (auto& e : m_model->enemies()) {
            if (!e.alive || e.exploding) continue;
            if (e.id != b.targetEnemyId) continue;
            const QPointF center = e.pos + QPointF(kEnemySize * 0.5, kEnemySize * 0.5);
            const QPointF d = center - b.pos;
            const qreal dist2 = d.x() * d.x() + d.y() * d.y();
            if (dist2 <= kHitRadius * kHitRadius) {
                b.alive = false;
                e.exploding = true;
                e.explosionFrame = 0;
                m_model->addScore(1);
                SpaceAudio::instance().play(SpaceAudioEvent::Blast);
                break;
            }
        }
    }
}

void SpaceController::upgradeDifficulty() {
    if (!m_model) return;
    if (m_model->state() != SpaceGameState::Playing) return;

    // simple upgrade: +1 maxEnemies up to 10, +1 speedLevel up to 10, faster spawn
    SpaceSettings& s = m_model->settings();
    s.maxEnemies = qMin(10, s.maxEnemies + 1);
    s.speedLevel = qMin(10, s.speedLevel + 1);
    const int newSpawn = qMax(250, kSpawnMsBase - s.speedLevel * 40);
    m_spawnTimer.start(newSpawn);
    SpaceAudio::instance().play(SpaceAudioEvent::Upgrade);
}

void SpaceController::spawnRewardWord() {
    if (!m_model) return;
    if (m_model->state() != SpaceGameState::Playing) return;
    if (!m_model->settings().rewardMode) return;
    m_wordProvider->requestWord();
}

void SpaceController::onWordReady(const QString& word, bool /*fromApi*/) {
    if (!m_model) return;
    if (m_model->state() != SpaceGameState::Playing) return;
    RewardWord& rw = m_model->rewardWord();
    rw.word = word;
    // 从左侧飞入；Y 在垂直方向中间 3/4（上下各留 1/8）内随机
    const int h = m_view ? m_view->height() : 600;
    qreal yMin = h / 8.0;
    qreal yMax = h * 7.0 / 8.0;
    if (yMax < yMin) {
        yMax = yMin;
    }
    const qreal wordY = yMin + QRandomGenerator::global()->generateDouble() * (yMax - yMin);
    rw.pos = QPointF(-10, wordY);
    rw.active = true;
    m_model->typedRewardBuffer().clear();
    SpaceAudio::instance().play(SpaceAudioEvent::WordOut);
}

/// 奖励词水平漂移，飞出右边界后清空输入缓冲。
void SpaceController::updateReward(double dt) {
    if (!m_model) return;
    if (!m_model->settings().rewardMode) return;
    RewardWord& rw = m_model->rewardWord();
    if (!rw.active) return;
    rw.pos.rx() += rw.speed * dt;
    if (rw.pos.x() > m_view->width() + 30) {
        rw.active = false;
        m_model->typedRewardBuffer().clear();
    }
}

/// 前缀不匹配则清空缓冲；完整匹配单词则生命回满并关闭当前奖励词。
void SpaceController::onRewardTyped(QChar c) {
    if (!m_model) return;
    if (!m_model->settings().rewardMode) return;
    RewardWord& rw = m_model->rewardWord();
    if (!rw.active) return;

    QString& buf = m_model->typedRewardBuffer();
    buf.push_back(c);
    if (!rw.word.startsWith(buf, Qt::CaseInsensitive)) {
        buf.clear();
        return;
    }
    if (buf.compare(rw.word, Qt::CaseInsensitive) == 0) {
        // reward success
        m_model->setLife(18);
        rw.active = false;
        buf.clear();
    }
}

