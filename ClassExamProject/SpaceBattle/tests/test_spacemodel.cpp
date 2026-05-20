/// @file test_spacemodel.cpp
/// SpaceModel 状态、reset、敌人/子弹与奖励词字段的单元测试（gtest）。

#include "Model/SpaceModel.h"

#include <gtest/gtest.h>

TEST(SpaceModel, initialState) {
    SpaceModel m;
    EXPECT_EQ(m.state(), SpaceGameState::Initial);
    EXPECT_EQ(m.score(), 0);
    EXPECT_EQ(m.life(), 18);
    EXPECT_DOUBLE_EQ(m.elapsedSec(), 0.0);
    EXPECT_EQ(m.shipAnimFrame(), 0);
    EXPECT_TRUE(m.enemies().isEmpty());
    EXPECT_TRUE(m.bullets().isEmpty());
    EXPECT_FALSE(m.rewardWord().active);
    EXPECT_TRUE(m.typedRewardBuffer().isEmpty());
}

TEST(SpaceModel, defaultSettings) {
    SpaceModel m;
    EXPECT_EQ(m.settings().maxEnemies, 5);
    EXPECT_EQ(m.settings().speedLevel, 5);
    EXPECT_EQ(m.settings().upgradeIntervalSec, 20);
    EXPECT_FALSE(m.settings().rewardMode);
}

TEST(SpaceModel, scoreOperations) {
    SpaceModel m;
    m.addScore(10);
    EXPECT_EQ(m.score(), 10);
    m.addScore(-5);
    EXPECT_EQ(m.score(), 5);
}

TEST(SpaceModel, lifeOperations) {
    SpaceModel m;
    m.setLife(10);
    EXPECT_EQ(m.life(), 10);
    m.setLife(0);
    EXPECT_EQ(m.life(), 0);
}

TEST(SpaceModel, elapsedTime) {
    SpaceModel m;
    m.addElapsed(1.5);
    EXPECT_DOUBLE_EQ(m.elapsedSec(), 1.5);
    m.addElapsed(-1.5);
    EXPECT_DOUBLE_EQ(m.elapsedSec(), 0.0);
}

TEST(SpaceModel, shipPosition) {
    SpaceModel m;
    m.setShipPos(QPointF(100, 200));
    EXPECT_DOUBLE_EQ(m.shipPos().x(), 100.0);
    EXPECT_DOUBLE_EQ(m.shipPos().y(), 200.0);
}

TEST(SpaceModel, shipVelocity) {
    SpaceModel m;
    EXPECT_DOUBLE_EQ(m.shipVel().x(), 160.0);
    EXPECT_DOUBLE_EQ(m.shipVel().y(), 0.0);

    m.setShipVel(QPointF(-100, 50));
    EXPECT_DOUBLE_EQ(m.shipVel().x(), -100.0);
    EXPECT_DOUBLE_EQ(m.shipVel().y(), 50.0);
}

TEST(SpaceModel, animFrameWrapping) {
    SpaceModel m;
    for (int i = 0; i < 5; ++i) {
        m.advanceShipAnimFrame();
    }
    EXPECT_EQ(m.shipAnimFrame(), 5);

    // wrap at 11
    m.setShipAnimFrame(0);
    for (int i = 0; i < 11; ++i) {
        m.advanceShipAnimFrame();
    }
    EXPECT_EQ(m.shipAnimFrame(), 0);

    // set beyond 11
    m.setShipAnimFrame(15);
    EXPECT_EQ(m.shipAnimFrame(), 4); // 15 % 11 = 4
}

TEST(SpaceModel, resetClearsRuntimeData) {
    SpaceModel m;
    m.addScore(50);
    m.setLife(5);
    m.addElapsed(30.0);
    m.setShipPos(QPointF(300, 400));

    SpaceEnemy e;
    e.id = 1;
    e.letter = 'A';
    m.enemies().push_back(e);

    SpaceBullet b;
    b.targetEnemyId = 1;
    m.bullets().push_back(b);

    m.typedRewardBuffer().append("hel");
    m.rewardWord().word = "hello";
    m.rewardWord().active = true;

    m.reset();

    EXPECT_EQ(m.state(), SpaceGameState::Initial);
    EXPECT_EQ(m.score(), 0);
    EXPECT_EQ(m.life(), 18);
    EXPECT_DOUBLE_EQ(m.elapsedSec(), 0.0);
    EXPECT_EQ(m.shipAnimFrame(), 0);
    EXPECT_TRUE(m.enemies().isEmpty());
    EXPECT_TRUE(m.bullets().isEmpty());
    EXPECT_FALSE(m.rewardWord().active);
    EXPECT_TRUE(m.typedRewardBuffer().isEmpty());
}

TEST(SpaceModel, resetPreservesSettings) {
    SpaceModel m;
    m.settings().maxEnemies = 8;
    m.settings().speedLevel = 3;
    m.settings().rewardMode = true;

    m.reset();

    EXPECT_EQ(m.settings().maxEnemies, 8);
    EXPECT_EQ(m.settings().speedLevel, 3);
    EXPECT_TRUE(m.settings().rewardMode);
}

TEST(SpaceModel, stateTransitions) {
    SpaceModel m;
    m.setState(SpaceGameState::Playing);
    EXPECT_EQ(m.state(), SpaceGameState::Playing);

    m.setState(SpaceGameState::Paused);
    EXPECT_EQ(m.state(), SpaceGameState::Paused);

    m.setState(SpaceGameState::GameOver);
    EXPECT_EQ(m.state(), SpaceGameState::GameOver);

    m.setState(SpaceGameState::End);
    EXPECT_EQ(m.state(), SpaceGameState::End);

    m.setState(SpaceGameState::Initial);
    EXPECT_EQ(m.state(), SpaceGameState::Initial);
}

TEST(SpaceModel, rewardWord) {
    SpaceModel m;
    m.rewardWord().word = "galaxy";
    m.rewardWord().pos = QPointF(100, 200);
    m.rewardWord().active = true;

    EXPECT_EQ(m.rewardWord().word, "galaxy");
    EXPECT_DOUBLE_EQ(m.rewardWord().pos.x(), 100.0);
    EXPECT_TRUE(m.rewardWord().active);

    m.typedRewardBuffer().append("gal");
    EXPECT_EQ(m.typedRewardBuffer(), "gal");
}

TEST(SpaceModel, enemyAndBulletContainers) {
    SpaceModel m;

    SpaceEnemy e1;
    e1.id = 1; e1.letter = 'A';
    m.enemies().push_back(e1);

    SpaceEnemy e2;
    e2.id = 2; e2.letter = 'B';
    m.enemies().push_back(e2);

    EXPECT_EQ(m.enemies().size(), 2);
    EXPECT_EQ(m.enemies()[0].letter, 'A');
    EXPECT_EQ(m.enemies()[1].letter, 'B');

    SpaceBullet b;
    b.targetEnemyId = 1;
    m.bullets().push_back(b);

    EXPECT_EQ(m.bullets().size(), 1);
    EXPECT_EQ(m.bullets()[0].targetEnemyId, 1);
}
