#include "cutouts.h"
#include <QString>
#include <QPoint>

#define _T(name, path, count, anchor, tiles, threshold) \
    std::pair<const QString, const FeatureDesc> \
        (name, \
         FeatureDesc(QString(":/cutouts/") + path, count, threshold, anchor, tiles))

typedef QPoint _P;

// VERY IMPORTANT: all images must be cut out from the same scale; currently 1920x1200 default zoom
// ALSO IMPORTANT: UI may not scale proportionally to map features, keep in separate FeatureList

// FIXME: anchor points
const FeatureDescList cutouts {
//    _T("TH9", "TH9.png", 1, 0.9), // different scale
    _T("TH10", "TH10.png", 1, _P(0, 0), 3, 0.9),
    _T("AD8", "defense/ad8.png", 0, _P(0, 0), 3, 0.8),
    _T("TR0", "props/TR0.png", 0, _P(0, 0), 2, 0.95),
    _T("SH0", "props/sh0.png", 0, _P(0, 0), 2, 0.95),
};

