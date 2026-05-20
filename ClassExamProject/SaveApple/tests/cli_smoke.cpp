#include "Config/GameConfig.h"
#include "Model/Fruit.h"
#include "Model/GameData.h"

#include <QCoreApplication>

#include <iostream>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    GameData data;
    if (data.lives() != 3 || data.score() != 0) {
        std::cerr << "GameData default values check failed\n";
        return 1;
    }

    data.addScore(10);
    if (data.score() != 10) {
        std::cerr << "GameData addScore check failed\n";
        return 2;
    }

    Fruit fruit(FruitType::GoodApple, QPointF(0, 0), 100.0);
    fruit.update(0.1f);
    if (fruit.position().y() <= 0.0) {
        std::cerr << "Fruit update check failed\n";
        return 3;
    }

    const QString bg = GameConfig::instance().imagePath("APPLE_BACKGROUND.png");
    if (bg.isEmpty()) {
        std::cerr << "GameConfig resource path check failed\n";
        return 4;
    }

    std::cout << "saveapple_cli_smoke: PASS\n";
    return 0;
}

