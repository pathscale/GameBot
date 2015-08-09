#include "botprogram.h"

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
    this->battlefield = new CocBattlefield(url.toLocalFile(), &sigs);

    emit this->heatmapChanged(this->battlefield->analyze());
}

BotProgram::~BotProgram() {
    if (this->battlefield) {
        delete this->battlefield;
    }
}
