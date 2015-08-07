#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>

#include "botprogram.h"
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qmlRegisterType<BotProgram>("CocBot", 1,0, "CocBot");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
