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
    const QPoint pos;
    const float value; // if methods other than TM_CCORR_NORMED are ever used, value should be normalized to reflect accuracy and not raw match
    Match(const int x, const int y, const float value)
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
    const Sprite *sprite;
    const Match match;
    FeatureMatch(const Feature *ftr, const Sprite *sprite, const Match &match)
        : ftr(ftr),
          sprite(sprite),
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

class Grid { // grid info in the default screenshot projection (rot45, known x/y ratio)
public:
    const float horzDist = 42; // horizontal pixel distance between 2 consecutive grid lines (empirical)
    const float vertDist = 31.5; // vertical (for slope)
    // const int size = 0; // number of tiles from origin (currently not needed, therefore infinite)
    const QPoint origin; // offset
    Grid(const QPoint &origin)
        : origin(origin)
    {}
};

class ResourceTuple
{
public:
    int gold;
    int elixir;
    int dark_elixir;

    ResourceTuple(int gold_v=-1, int elixir_v=-1, int dark_elixir_v=-1)
        : gold(gold_v), elixir(elixir_v), dark_elixir(dark_elixir_v)
    {}
};

class CocBattlefield
{
    const cv::Mat screen;
    ResourceManager *buildings;
    BattlefieldSignals *sig;
    ResourceTuple available_loot;
    std::list<FeatureMatch> defense_buildings;
    Grid *grid = NULL;
public:
    CocBattlefield(const QString &filepath, ResourceManager *buildings, BattlefieldSignals *proxy=NULL);
    const std::list<FeatureMatch> analyze();
    ~CocBattlefield();

    inline double getScale() {
        return buildings->getScale();
    }
    // TODO: QPoint screen_to_grid(QPoint) - when grid gains defined borders
protected:
    double find_scale();
    void find_grid();
    void draw_grid();
    const std::list<FeatureMatch> find_buildings();

    // this may be more useful as std::map<QPoint, Defense>
    // QPoint describes bitmap position, not grid coords
    const std::list<std::pair<QPoint, const Defense &> > find_defenses(const std::list<FeatureMatch> &buildings);
    void find_loot_numbers(float threshold=0.8);
};

#endif // COCBATTLEFIELD_H
