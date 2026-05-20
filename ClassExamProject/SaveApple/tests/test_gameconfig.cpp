#include "Config/GameConfig.h"

#include <gtest/gtest.h>

TEST(GameConfigTest, ReturnsStableBasicSettings) {
    const GameConfig& cfg = GameConfig::instance();
    EXPECT_EQ(cfg.initialLives(), 3);
    EXPECT_EQ(cfg.spawnIntervalMs(), 900);
    EXPECT_EQ(cfg.updateIntervalMs(), 16);
}

TEST(GameConfigTest, ResolvesKnownImagePath) {
    const GameConfig& cfg = GameConfig::instance();
    const QString path = cfg.imagePath("APPLE_BACKGROUND.png");
    EXPECT_FALSE(path.isEmpty());
}

