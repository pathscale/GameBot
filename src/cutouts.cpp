#include "cutouts.h"

#define _T(name, path, count, threshold) std::pair<const QString, const FeatureDesc>(name, FeatureDesc(QString(":/cutouts/") + path, count, threshold))

// VERY IMPORTANT: all images must be cut out from the same scale; currently 1920x1200 default zoom
// ALSO IMPORTANT: UI may not scale proportionally to map features, keep in separate FeatureList

const FeatureDescList cutouts {
    _T("TH9", "TH9.png", 1, 0.9),
    _T("FW0", "defense/fw0.png", 0, 0.8),
    _T("TR0", "props/TR0.png", 0, 0.95),
};

