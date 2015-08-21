#ifndef COCBATTLEFIELD_H
#define COCBATTLEFIELD_H

#include <QObject>
#include <QPoint>
#include <QString>
#include <QRect>
#include <QFile>
#include <QDataStream>

#include <opencv2/imgproc/imgproc.hpp>

#include "resourcemanager.h"

// Program logic. Only basic Qt types allowed (TODO)

class Match {
public:
    QPoint pos;
    float value; // if methods other than TM_CCORR_NORMED are ever used, value should be normalized to reflect accuracy and not raw match
    Match(int x, int y, float value)
        : pos(x, y), value(value)
    {}
    Match(const Match &m)
        : pos(m.pos),
          value(m.value)
    {}
};

class FeatureMatch {
public:
    const Feature *ftr;
    const Match match;
    FeatureMatch(const Feature *ftr, const Match &match)
        : ftr(ftr),
          match(match)
    {}
};

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
    const std::list<FeatureMatch> analyze();
public:
    inline double getScale() {
        return buildings->getScale();
    }
protected:
    double find_scale();
};

#endif // COCBATTLEFIELD_H
