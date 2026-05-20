/* -------------------------------------------------------------------------
//  文件名    : main.cpp
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : TypeGame 统一入口——路由到 GUI 启动器 / 单游戏 / CLI 测试
// -------------------------------------------------------------------------*/

#include "GameLauncher.h"
#include "cli_mode.h"
#include "gui_mode.h"

#include <QApplication>
#include <QCommandLineParser>

#include <cstdio>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("TypeGame");
    QApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("Unified launcher: apple | space. --test for CLI mode.");
    parser.addHelpOption(); parser.addVersionOption();
    parser.addPositionalArgument("game", "apple | space");
    QCommandLineOption testOpt(QStringList{"t", "test"}, "CLI test mode (requires --input and --output).");
    QCommandLineOption inOpt("input", "Input JSON config.", "file");
    QCommandLineOption outOpt("output", "Output JSON result.", "file");
    parser.addOption(testOpt); parser.addOption(inOpt); parser.addOption(outOpt);
    parser.process(app);

    const QStringList pos = parser.positionalArguments();

    // No args → GUI launcher
    if (pos.isEmpty()) {
        GameLauncher w;
        QObject::connect(&w, &GameLauncher::gameSelected, [](int idx) {
            if (idx == 0) runAppleGui();
            else if (idx == 1) runSpaceGui();
        });
        w.show();
        return app.exec();
    }

    const QString game = pos.at(0).toLower();

    if (parser.isSet(testOpt)) {
        if (!parser.isSet(inOpt) || !parser.isSet(outOpt)) return 1;
        if (game == "space") return runSpaceCliTest(parser.value(inOpt), parser.value(outOpt));
        if (game == "apple") return runAppleCliTest(parser.value(inOpt), parser.value(outOpt));
        return 1;
    }

    if (game == "apple") { runAppleGui(); return app.exec(); }
    if (game == "space") { runSpaceGui(); return app.exec(); }

    std::fprintf(stderr, "typegame: game must be apple or space\n");
    return 1;
}
