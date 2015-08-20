#include "botprogram.h"
#include <functional>

#include <QQmlEngine>
#include <QQmlComponent>
#include <QDebug>
BotProgram::BotProgram(QObject *parent)
    : QObject(parent),
      sigs(this)
{
    connect(&sigs, SIGNAL(debugChanged(QString)), this, SLOT(onDebugChanged(QString)));
}

void BotProgram::onDebugChanged(const QString &filename) {
    emit debugChanged(QUrl::fromLocalFile(filename));
}

void BotProgram::loadUrl(const QUrl &url) {
    this->battlefield = new CocBattlefield(url.toLocalFile(), &buildings, &sigs);
    const std::list<FeatureMatch>f = this->battlefield->analyze();

    QList<QObject*> boxen;

    for (const FeatureMatch &fm : f) {
        const Feature *f = fm.ftr;
        const Match &m = fm.match;
        QQmlEngine engine;
        QQmlComponent component(&engine,
                QUrl(QStringLiteral("qrc:/MatchBox.qml")));
        QObject *o = component.create(); // data container compatible with QML that will expose all needed properties
        o->setProperty("x", m.pos.x());
        o->setProperty("y", m.pos.y());
        o->setProperty("width", f->img.size().width);
        o->setProperty("height", f->img.size().height);
        o->setProperty("fit", QString::number(m.value));
        boxen.append(o);
    }

    emit this->heatmapChanged(boxen);
    for (const QObject *box : boxen) {
        delete box;
    }
}

BotProgram::~BotProgram() {
    if (this->battlefield) {
        delete this->battlefield;
    }
}
