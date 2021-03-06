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
#include "detection.h"
#include "objects.h"

// Program logic. Only basic Qt types allowed (TODO)

class BattlefieldSignals : public QObject
{
    Q_OBJECT
public:
    explicit BattlefieldSignals(QObject *parent = 0);
signals:
    void imageChanged(const QString &filename);
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
// TODO: separate algorithm from state & make state const-public
class CocBattlefield
{
    const cv::Mat screen;
    FeatureManager *buildings;
    BattlefieldSignals *sig;
    ResourceTuple available_loot;
    std::list<std::pair<QPoint, const Defense*>> defense_buildings;
    Grid *grid = NULL;
public:
    CocBattlefield(const QString &filepath, FeatureManager *buildings, BattlefieldSignals *proxy=NULL);
    const std::list<FeatureMatch> analyze();
    ~CocBattlefield();
    inline const std::list<std::pair<QPoint, const Defense*>> &get_defense_buildings() {
        return defense_buildings;
    }
    inline double getScale() {
        return buildings->getScale();
    }
    const DamageValue get_pixel_damage(const QPoint &pixel) const;
    float get_tile_distance(const QPoint &p1, const QPoint &p2) const;
    const QPointF screen_to_gridF(const QPoint &p) const;
    // TODO: QPoint screen_to_grid(QPoint) - when grid gains defined borders
protected:
    // returns pixels on the screen; TODO: return tile coords
    const QPoint get_building_center(const int tileWidth);
    double find_scale();
    void find_grid();
    void draw_grid();
    const std::list<FeatureMatch> find_buildings();

    // this may be more useful as std::map<QPoint, Defense>
    // QPoint describes bitmap position, not grid coords
    void find_defenses(const std::list<FeatureMatch> &buildings);
    void find_loot_numbers(float threshold=0.8);
};

#endif // COCBATTLEFIELD_H
