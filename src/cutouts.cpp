#include "objects.h"
#include "cutouts.h"
#include <QString>
#include <QPoint>

// F for "feature"
// use for all props that are not moving & tied to the level grid
// name - handle to refer to feature in code
// humanName - name for debugging
// path - path to image file relative to qrc:/cutouts/
// threshold - detection threshold
// anchor - QPoint [alias pos] for pixel difference (top tile top corner - image top-left corner)
// tiles - size in tiles (both dimensions assumed equal)
// type - building type class
#define _F(name, humanName, path, threshold, count, anchor, tiles, type) \
    std::pair<const QString, const FeatureDesc> \
        (name, \
         FeatureDesc(QString(":/cutouts/") + path, humanName, count, threshold, anchor, tiles, type))

typedef QPoint pos;

// VERY IMPORTANT: all images must be cut out from the same scale; currently 1920x1200 default zoom
// ALSO IMPORTANT: UI may not scale proportionally to map features, keep in separate FeatureList

const FeatureDescList cutouts {
//    _F("TH9", "TH9.png", 1, 0.9), // different scale
    _F("TH10",  "TownHall10",   "TH10.png",         0.9,    1, pos(25, 12),     4, Building(5500)),
    _F("AD8",   "AirDefense8",  "defense/ad8.png",  0.8,    0, pos(17, -8),     3, Defense(1170, 10, 320, Defense::SINGLE, Defense::AIR)),
    _F("TR0",   "Tree",         "props/TR0.png",    0.95,   0, pos(20, -15),    2, Scenery()),
    _F("SH0",   "Shed",         "props/sh0.png",    0.95,   0, pos(20, -3),     2, Scenery()),
};
