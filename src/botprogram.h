#ifndef BOTPROGRAM_H
#define BOTPROGRAM_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QRect>
#include "cocbattlefield.h"
#include "resourcemanager.h"

/** This class embodies the whole program, UI and all */

class BotProgram : public QObject
{
    Q_OBJECT
    QString url;
    CocBattlefield *battlefield = NULL;
    BattlefieldSignals sigs;
    ResourceManager buildings = ResourceManager(":/cutouts");
public:
    explicit BotProgram(QObject *parent = 0);
    Q_INVOKABLE void loadUrl(const QUrl &url);
    ~BotProgram();
signals:
    void heatmapChanged(const QVariantList &dmg);
    void debugChanged(const QUrl &url);
public slots:
    void onDebugChanged(const QString &filename);
};

#endif // BOTPROGRAM_H
