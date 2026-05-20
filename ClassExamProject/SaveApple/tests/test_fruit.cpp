#include "Model/Fruit.h"

#include <gtest/gtest.h>

TEST(FruitTest, UpdateMovesFruitBySpeedAndDelta) {
    Fruit fruit(FruitType::GoodApple, QPointF(10, 20), 100.0);
    fruit.update(0.5f);
    EXPECT_NEAR(fruit.position().x(), 10.0, 1e-6);
    EXPECT_NEAR(fruit.position().y(), 70.0, 1e-6);
}

TEST(FruitTest, ScoreValueMatchesType) {
    Fruit good(FruitType::GoodApple, QPointF(), 1.0);
    Fruit small(FruitType::SmallApple, QPointF(), 1.0);
    Fruit bad(FruitType::BadApple, QPointF(), 1.0);
    EXPECT_EQ(good.scoreValue(), 10);
    EXPECT_EQ(small.scoreValue(), 5);
    EXPECT_EQ(bad.scoreValue(), 0);
}

TEST(FruitTest, SmashFrameTicksToZero) {
    Fruit fruit(FruitType::GoodApple, QPointF(), 1.0);
    fruit.setSmashFrames(2);
    fruit.tickSmashFrame();
    fruit.tickSmashFrame();
    fruit.tickSmashFrame();
    EXPECT_EQ(fruit.smashFrames(), 0);
}

