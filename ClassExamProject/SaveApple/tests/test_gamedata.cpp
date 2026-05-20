#include "Model/GameData.h"

#include <gtest/gtest.h>

TEST(GameDataTest, DefaultValuesAreExpected) {
    GameData data;
    EXPECT_EQ(data.score(), 0);
    EXPECT_EQ(data.lives(), 3);
    EXPECT_FALSE(data.isGameOver());
    EXPECT_FALSE(data.isPassed());
}

TEST(GameDataTest, LoseLifeTriggersGameOverAtZero) {
    GameData data;
    data.loseLife();
    data.loseLife();
    data.loseLife();
    data.loseLife(); // extra call should stay at 0
    EXPECT_EQ(data.lives(), 0);
    EXPECT_TRUE(data.isGameOver());
}

TEST(GameDataTest, BoundSettingsAreClamped) {
    GameData data;
    data.setFallSpeed(100);
    data.setMaxAppleCount(-1);
    data.setTargetCount(2);
    EXPECT_EQ(data.fallSpeed(), 10);
    EXPECT_EQ(data.maxAppleCount(), 1);
    EXPECT_EQ(data.targetCount(), 5);
}

TEST(GameDataTest, RecordHitMarksPassedWhenTargetReached) {
    GameData data;
    data.setTargetCount(5);
    for (int i = 0; i < 5; ++i) {
        data.recordHit();
    }
    EXPECT_TRUE(data.isPassed());
}

