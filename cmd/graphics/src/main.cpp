#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QScreen>
#include <iostream>
#include <QSettings>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument("ip", "ip addr");
    parser.process(a);

    auto positionalArgs = parser.positionalArguments();
    if (positionalArgs.size() != 1) {
        std::cerr << "Not enough arguments. Please provide ip addr" << std::endl;
        a.exit(1);
        return 1;
    }

    QCoreApplication::setApplicationName("Projects 5 & 6: Lights, Camera & Action!");
    QCoreApplication::setOrganizationName("CS 1230");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    QSurfaceFormat fmt;
    fmt.setVersion(4, 1);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(fmt);

    MainWindow w;
    w.initialize(positionalArgs[0]);
    w.resize(800, 600);
    w.show();

    int return_val = a.exec();
    w.finish();
    return return_val;
}
