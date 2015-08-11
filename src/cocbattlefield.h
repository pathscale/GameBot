#ifndef COCBATTLEFIELD_H
#define COCBATTLEFIELD_H

#include <QObject>
#include <QString>
#include <QRect>
#include <QFile>
#include <QDataStream>

#include <opencv2/imgproc/imgproc.hpp>

#include "resourcemanager.h"

// Program logic. Only basic Qt types allowed (TODO)

class BattlefieldSignals : public QObject
{
    Q_OBJECT
public:
    explicit BattlefieldSignals(QObject *parent = 0);
signals:
    void debugChanged(const QString &filename);
};

class CocBattlefield
{
    cv::Mat screen;
    ResourceManager *buildings;
    BattlefieldSignals *sig;
public:
    CocBattlefield(const QString &filepath, ResourceManager *buildings, BattlefieldSignals *proxy=NULL);
    const QVariantList analyze();
};

#endif // COCBATTLEFIELD_H
