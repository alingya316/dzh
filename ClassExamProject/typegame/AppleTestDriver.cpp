/* -------------------------------------------------------------------------
//  文件名    : AppleTestDriver.cpp
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : 无 GUI 驱动 SaveApple 游戏，按字母序列模拟按键
// -------------------------------------------------------------------------*/

#include "AppleTestDriver.h"

#include "Controller/GameController.h"
#include "Model/GameData.h"
#include "View/GameView.h"

#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

QJsonObject AppleTestDriver::run(const QString& letters, int durationSec) {
    GameData model;
    GameView view(&model);
    view.setFixedSize(1024, 768); // virtual size
    GameController ctrl(&model, &view);

    ctrl.startGame();
    qint64 startMs = QDateTime::currentMSecsSinceEpoch();
    qint64 durationMs = durationSec * 1000;
    int letterIdx = 0;
    int totalInputs = 0, hits = 0;
    qint64 lastSpawnMs = 0, lastInputMs = 0;
    const int spawnIntervalMs = 900;

    while (true) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - startMs >= durationMs) break;
        if (model.isGameOver()) break;

        QApplication::processEvents();
        ctrl.updateGame();

        // Spawn fruits periodically
        if (now - lastSpawnMs >= spawnIntervalMs) {
            lastSpawnMs = now;
            ctrl.spawnFruit();
        }

        // Inject letters
        if (now - lastInputMs >= 400 && !letters.isEmpty() && letterIdx < letters.length()) {
            lastInputMs = now;
            QChar c = letters[letterIdx++];
            int prevScore = model.score();
            ctrl.onLetterPressed(c);
            if (model.score() > prevScore) hits++;
            totalInputs++;
        }
    }

    ctrl.onEndButtonClicked();
    QApplication::processEvents();

    QJsonObject root;
    root["allPassed"] = !model.isGameOver();
    root["gameName"] = "Apple";
    root["result"] = model.isGameOver() ? "failed" : "passed";

    QJsonArray rounds;
    QJsonObject r;
    r["round"] = 1;
    r["mode"] = "FromConfig";
    r["passed"] = !model.isGameOver();
    QJsonObject cmp;
    cmp["correctInputs"] = QString("%1/%1").arg(hits);
    cmp["score"] = QString("%1/%1").arg(model.score());
    cmp["wrongInputs"] = QString("%1/%1").arg(totalInputs - hits);
    r["comparison"] = cmp;
    r["health"] = QString("%1/%1").arg(model.lives()).arg(5);
    r["missed"] = totalInputs - hits;
    r["prizeText"] = QJsonObject();
    rounds.append(r);
    root["rounds"] = rounds;
    root["modeStats"] = QJsonObject();

    return root;
}

bool AppleTestDriver::saveResult(const QString& path, const QJsonObject& result) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    f.write(QJsonDocument(result).toJson(QJsonDocument::Indented));
    return true;
}
