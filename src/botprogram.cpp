#include "botprogram.h"

#include <QDebug>
BotProgram::BotProgram(QObject *parent) : QObject(parent)
{

}

void BotProgram::loadUrl(const QUrl &url) {
    this->battlefield = new CocBattlefield(url.toLocalFile());

    emit this->heatmapChanged(this->battlefield->analyze());
}

BotProgram::~BotProgram() {
    if (this->battlefield) {
        delete this->battlefield;
    }
}
