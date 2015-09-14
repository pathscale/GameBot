#ifndef BOTPROGRAM_H
#define BOTPROGRAM_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QRect>
#include "cocbattlefield.h"
#include "resourcemanager.h"
#include "cutouts.h"

/** This class embodies the whole program, UI and all */

class BotProgram : public QObject
{
    Q_OBJECT
    QString url;
    CocBattlefield *battlefield = NULL;
    BattlefieldSignals sigs;
    FeatureManager buildings = FeatureManager(cutouts);
public:
    explicit BotProgram(QObject *parent = 0);
    Q_INVOKABLE void loadUrl(const QUrl &url);
    Q_INVOKABLE QString getDamageText(int x, int y);
    ~BotProgram();
protected:
    void display_matches(const std::list<FeatureMatch> &fmlist);
    void display_heatmap(const std::list<std::pair<QPoint, const Defense *> > &buildings);
signals:
    void heatmapChanged(const QVariantList &dmg);
    void matchesChanged(const QVariantList &dmg);
    void debugChanged(const QList<QObject*> &dbg);
    void imageChanged(const QUrl &filename);
public slots:
    void onImageChanged(const QString &filename);
};

#endif // BOTPROGRAM_H
