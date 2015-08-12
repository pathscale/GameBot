#include "cutouts.h"

#define _T(name, path, count) std::pair<const QString, const FeatureDesc>(name, FeatureDesc(QString(":/cutouts/") + path, count))

const FeatureList cutouts {
    _T("TH9", "TH9.png", 1),
    _T("FW0", "defense/fw0.png", 0),
    _T("TR0", "props/TR0.png", 0),
};

