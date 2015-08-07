#ifndef BOTPROGRAM_H
#define BOTPROGRAM_H

#include <QObject>
#include <QUrl>
#include <QRect>
#include "cocbattlefield.h"

/** This class embodies the whole program, UI and all */

class BotProgram : public QObject
{
    Q_OBJECT
    QString url;
    CocBattlefield *battlefield = NULL;
public:
    explicit BotProgram(QObject *parent = 0);
    Q_INVOKABLE void loadUrl(const QUrl &url);
    ~BotProgram();
signals:
    void heatmapChanged(const QRect &dmg);
public slots:
};

#endif // BOTPROGRAM_H
