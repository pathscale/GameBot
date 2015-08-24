#include "objects.h"
#include "cutouts.h"
#include <QString>
#include <QPoint>

// F for "feature"
// use for all props that are not moving & tied to the level grid
// name - handle to refer to feature in code
// humanName - name for debugging
// sprite - sprite list, see below
// tiles - size in tiles (both dimensions assumed equal)
// type - building type class, see objects.h
#define _F(name, humanName, sprite, count, tiles, type) \
    std::pair<const QString, const FeatureDesc> \
        (name, \
         FeatureDesc(sprite, humanName, count, tiles, type))

// S for "sprites"
// The sprite list contains multiple ways in which the same feature may be detected (FIXME: allow overlapping).
// List contains multiple I entries

// I for "Image"
// path - path to image file relative to qrc:/cutouts/
// threshold - detection threshold
// anchor - QPoint [alias pos] for pixel difference (top tile top corner - image top-left corner)
#define _I(path, position, threshold) {QString(":/cutouts/") + path, position, threshold}
typedef SpriteDescAlts _S;
typedef QPoint pos;

// SINGLE can replace S when detection doesn't need multiple images
#define _SINGLE(path, position, threshold) _S({_I(path, position, threshold)})

// VERY IMPORTANT: all images must be cut out from the same scale; currently 1920x1200 default zoom
// ALSO IMPORTANT: UI may not scale proportionally to map features, keep in separate FeatureList

const FeatureDescList cutouts {
    _F("TH10",  "TownHall10",   _SINGLE("TH10.png",             pos(25, 12),    0.9),   1,  4, Building(5500)),
    _F("AD8",   "AirDefense8",  _SINGLE("defense/ad8.png",      pos(17, -8),    0.8),   0,  3, Defense(1170, 10, 320, Defense::SINGLE, Defense::AIR)),
    _F("TR0",   "Tree",         _SINGLE("props/TR0.png",        pos(20, -15),   0.95),  0,  2, Scenery()),
    _F("SH0",   "Shed",         _SINGLE("props/sh0.png",        pos(20, -3),    0.95),  0,  2, Scenery()),
    _F("G0-0",  "GoldStor0_10$",_SINGLE("storage/gold0-0.png",  pos(0, 0),      0.95),  0,  3, Storage(0, Storage::GOLD, 0)), // FIXME: health, offset, amount
    _F("AH0",   "AirSweeper0",  _SINGLE("defense/ah0.png",      pos(0, 0),      0.95),  0,  2, Defense(0, 0, 0, Defense::PUSH, Defense::AIR)), // FIXME
    _F("CA0",   "Cannon0",      _S({_I("defense/ca0.png",       pos(0, 0),      0.95),
                                    _I("defense/ca0_1.png",     pos(0, 0),      0.95)}),0,  3, Building(0)), // FIXME
};
