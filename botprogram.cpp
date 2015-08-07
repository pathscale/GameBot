#include "botprogram.h"

#include <QDebug>
BotProgram::BotProgram(QObject *parent) : QObject(parent)
{

}

void BotProgram::loadUrl(QUrl url) {
    qDebug() << "loaded " << url;
}
