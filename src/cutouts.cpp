#include "cutouts.h"
#include <QString>
#include <QPoint>

// F for "feature"
#define _F(name, path, count, anchor, tiles, threshold) \
    std::pair<const QString, const FeatureDesc> \
        (name, \
         FeatureDesc(QString(":/cutouts/") + path, count, threshold, anchor, tiles))

typedef QPoint pos;

// VERY IMPORTANT: all images must be cut out from the same scale; currently 1920x1200 default zoom
// ALSO IMPORTANT: UI may not scale proportionally to map features, keep in separate FeatureList

const FeatureDescList cutouts {
//    _F("TH9", "TH9.png", 1, 0.9), // different scale
    _F("TH10", "TH10.png", 1, pos(25, 12), 4, 0.9),
    _F("AD8", "defense/ad8.png", 0, pos(17, -8), -3, 0.8),
    _F("TR0", "props/TR0.png", 0, pos(20, -15), 2, 0.95),
    _F("SH0", "props/sh0.png", 0, pos(20, -3), 2, 0.95),
};

