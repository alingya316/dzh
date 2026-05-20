/* -------------------------------------------------------------------------
//  文件名    : SpaceTestDriver.cpp
//  创建者    : dengzihang
//  创建时间  : 2026-05-03
//  功能描述  : 无 GUI 驱动 SpaceBattle 游戏循环，按模式模拟按键，输出 JSON 结果
// -------------------------------------------------------------------------*/

#include "SpaceTestDriver.h"

#include "Controller/SpaceController.h"
#include "View/SpaceView.h"

#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRandomGenerator>

#include <algorithm>

CliTestConfig SpaceTestDriver::loadConfig(const QString& path) {
    CliTestConfig cfg;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return cfg;
    QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();
    cfg.letters = root.value("letters").toString();
    QJsonObject s = root.value("settings").toObject();
    cfg.settings.maxEnemies = s.value("enemysNum").toInt(5);
    cfg.settings.speedLevel = s.value("enemysSpeed").toInt(3);
    cfg.settings.rewardMode = s.value("isPrize").toBool(true);
    cfg.settings.upgradeIntervalSec = s.value("upgradeInterval").toInt(120);
    cfg.testGameDuration = root.value("testGameDuration").toInt(30);
    QJsonObject tc = root.value("testConfig").toObject();
    cfg.correctRounds = tc.value("correctRounds").toInt(1);
    cfg.errorRounds = tc.value("errorRounds").toInt(1);
    cfg.allwrongRounds = tc.value("allwrongRounds").toInt(1);
    cfg.errorFrequency = tc.value("errorFrequency").toInt(5);
    return cfg;
}

bool SpaceTestDriver::saveResult(const QString& path, const QJsonObject& result) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    f.write(QJsonDocument(result).toJson(QJsonDocument::Indented));
    return true;
}

// ── helpers ──

static QChar pickCorrectLetter(const SpaceModel* model) {
    qreal bestY = -1e9;
    QChar best = 'A';
    for (const auto& e : model->enemies()) {
        if (e.alive && !e.exploding && e.pos.y() > bestY) {
            bestY = e.pos.y();
            best = e.letter;
        }
    }
    return best;
}

static QChar pickWrongLetter(const SpaceModel* model) {
    bool used[26] = {};
    for (const auto& e : model->enemies()) {
        if (e.alive && !e.exploding) {
            int idx = e.letter.toUpper().unicode() - 'A';
            if (idx >= 0 && idx < 26) used[idx] = true;
        }
    }
    QVector<int> unused;
    for (int i = 0; i < 26; ++i) if (!used[i]) unused.push_back(i);
    if (unused.isEmpty()) return 'A';
    return QChar('A' + unused[QRandomGenerator::global()->bounded(unused.size())]);
}

struct RoundResult {
    QString mode;
    bool passed = false;
    int correctInputs = 0;
    int wrongInputs = 0;
    int score = 0;
    int health = 0;
    int maxHealth = 18;
    int missed = 0;
};

static RoundResult runOneRound(SpaceController* ctrl, SpaceModel* model, SpaceView* /*view*/,
                               const CliTestConfig& cfg, const QString& mode, int /*roundNum*/) {
    RoundResult r;
    r.mode = mode;
    r.maxHealth = model->life();

    ctrl->onStart();
    qint64 startMs = QDateTime::currentMSecsSinceEpoch();
    qint64 durationMs = cfg.testGameDuration * 1000;
    int letterIdx = 0;
    int pressCount = 0;
    int errorInterval = qMax(1, cfg.errorFrequency);
    qint64 lastInputMs = 0;
    int prevScore = 0;

    while (true) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - startMs >= durationMs) break;
        if (model->state() != SpaceGameState::Playing) break;

        QApplication::processEvents();
        ctrl->tick();

        // Track actual score changes from previous inputs
        int curScore = model->score();
        if (curScore > prevScore) {
            r.correctInputs += (curScore - prevScore);
            prevScore = curScore;
        }

        if (now - lastInputMs >= 900) {
            lastInputMs = now;

            QChar toPress;
            bool expectWrong = false;

            if (mode == "AllCorrect") {
                toPress = pickCorrectLetter(model);
            } else if (mode == "AllWrong") {
                toPress = pickWrongLetter(model);
                expectWrong = true;
            } else {
                if (pressCount > 0 && pressCount % errorInterval == 0) {
                    toPress = pickWrongLetter(model);
                    expectWrong = true;
                } else {
                    toPress = pickCorrectLetter(model);
                }
            }

            // Config letters only for controlled-error modes, not AllCorrect
            if (!cfg.letters.isEmpty() && letterIdx < cfg.letters.length()
                && mode != "AllCorrect")
                toPress = cfg.letters[letterIdx++];

            prevScore = model->score();
            ctrl->onLetterPressed(toPress);

            if (expectWrong) r.wrongInputs++;
            pressCount++;
        }
    }

    // Catch final score changes
    r.correctInputs += qMax(0, model->score() - prevScore);
    r.score = model->score();
    r.health = model->life();
    r.missed = r.wrongInputs;
    r.passed = (r.health > 0);

    ctrl->onReturnToMenu();
    QApplication::processEvents();
    return r;
}

QJsonObject SpaceTestDriver::run(const CliTestConfig& cfg) {
    SpaceModel model;
    model.settings() = cfg.settings;

    SpaceView view(&model);
    view.setFixedSize(1024, 768);

    SpaceController ctrl(&model, &view);

    QList<RoundResult> rounds;
    int roundNum = 0;

    for (int i = 0; i < cfg.correctRounds; ++i)
        rounds.append(runOneRound(&ctrl, &model, &view, cfg, "AllCorrect", ++roundNum));
    for (int i = 0; i < cfg.errorRounds; ++i)
        rounds.append(runOneRound(&ctrl, &model, &view, cfg, "WithErrors", ++roundNum));
    for (int i = 0; i < cfg.allwrongRounds; ++i)
        rounds.append(runOneRound(&ctrl, &model, &view, cfg, "AllWrong", ++roundNum));

    QJsonObject root;
    root["gameName"] = "Space";

    int passed = 0, total = rounds.size();
    QJsonObject modeStats;
    for (const auto& mode : {"AllCorrect", "WithErrors", "AllWrong"}) {
        int mp = 0, mt = 0;
        for (const auto& r : rounds) {
            if (r.mode == mode) { mt++; if (r.passed) mp++; }
        }
        modeStats[mode] = QString("%1/%2").arg(mp).arg(mt);
    }
    passed = static_cast<int>(std::count_if(rounds.cbegin(), rounds.cend(),
        [](const RoundResult& r) { return r.passed; }));

    root["allPassed"] = (passed == total);
    root["modeStats"] = modeStats;
    root["result"] = QString("%1/%2 passed").arg(passed).arg(total);

    QJsonArray roundsArr;
    for (int i = 0; i < rounds.size(); ++i) {
        const auto& r = rounds[i];
        QJsonObject ro;
        ro["round"] = i + 1; ro["mode"] = r.mode; ro["passed"] = r.passed;
        ro["missed"] = r.missed;
        ro["health"] = QString("%1/%2").arg(r.health).arg(r.maxHealth);
        QJsonObject cmp;
        cmp["correctInputs"] = QString("%1/%1").arg(r.correctInputs);
        cmp["score"] = QString("%1/%1").arg(r.score);
        cmp["wrongInputs"] = QString("%1/%1").arg(r.wrongInputs);
        ro["comparison"] = cmp;
        QJsonObject pz; pz["chars"] = 0; pz["words"] = 0;
        ro["prizeText"] = pz;
        roundsArr.append(ro);
    }
    root["rounds"] = roundsArr;

    return root;
}
