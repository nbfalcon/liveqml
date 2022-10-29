#include <QApplication>
#include <QQmlEngine>
#include <QCommandLineParser>
#include "liveqml.h"


int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);
    app.setApplicationName("LiveQML");
    app.setApplicationVersion("0.1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("QML Live Preview");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", app.translate("file", "File to load\n"));
    parser.process(app);

    auto files = parser.positionalArguments();

    qDebug() << "CWD:" << QDir::currentPath();

    QQmlEngine engine;
    engine.addImportPath(QDir::currentPath());
    qtx::LiveQML liveEngine{&engine};
    liveEngine.setBaseDirectory(QDir::current());
    for (const auto &file : files) {
        liveEngine.load(file);
    }

    return app.exec();
}
