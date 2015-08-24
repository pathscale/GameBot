#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "objects.h"
#include <QString>
#include <QPoint>
#include <QDebug>
#include <opencv2/imgproc/imgproc.hpp>

class SpriteDesc {
public:
    const QString filename;
    const QPoint anchor; // position of top corner of the tiles in screen pixels relative to top left corner of image
    const float detectionThreshold;
};

typedef std::list<SpriteDesc> SpriteDescAlts;

class Sprite;

typedef std::list<Sprite> SpriteAlts;

class Sprite {
public:
    const cv::Mat img;
    const QPoint anchor; // position of top corner of the tiles in screen pixels relative to top left corner of image
    const float detectionThreshold;
    inline Sprite(const cv::Mat &img, const QPoint &anchor, const float threshold)
        : img(img),
          anchor(anchor),
          detectionThreshold(threshold)
    {}
    static const SpriteAlts alts_from_descs(const SpriteDescAlts &descs);
    static const SpriteAlts scale_alts(const SpriteAlts &sprites, double scale);
};

class FeatureBase {
public:
    const QString humanName;
    const int maxCount;
    const int tileWidth; // width in tiles
    const ObjectBase type;
protected:
    inline FeatureBase(const QString &humanName, const int maxCount, const int tileWidth, const ObjectBase &type)
        : humanName(humanName),
          maxCount(maxCount),
          tileWidth(tileWidth),
          type(type)
    {}
};

/* TemplData != Template because:
 * 1. data must be immutable
 * 2. loading images must be done after Qt resource system initializes
 * 3. if lazy loading is used, it will cut into time allocated for processing - unacceptable
 */

class FeatureDesc : public FeatureBase {
public:
    const SpriteDescAlts sprites;
    inline FeatureDesc(const SpriteDescAlts &sprites, const QString &humanName, const int maxCount, const int tileWidth, const ObjectBase &type)
        : FeatureBase(humanName, maxCount, tileWidth, type),
          sprites(sprites)
    {}
};

typedef std::list<std::pair<const QString, const FeatureDesc>> FeatureDescList; // TODO: std::pair should be const, but how to do it?

class Feature : public FeatureBase
{
public:
    const SpriteAlts sprites;
    Feature(const SpriteAlts &sprites, const QString humanName, const int maxCount, const int tileWidth, const ObjectBase &type);
    Feature(const FeatureDesc &td);
    Feature scaled(double scale) const;
};

typedef std::map<const QString, Feature> FeatureMap; // loaded object

inline FeatureMap load_from_ftrs(const FeatureDescList &features) {
    // Don't use before Qt resource system is initialized!
    FeatureMap ret;
    // does resource loading by converting FeatureDesc into Template objects (actual loading in constructor)
    for (const std::pair<const QString, const FeatureDesc> ftr_pair : features) {
        const std::pair<const QString, Feature> loaded(ftr_pair.first, ftr_pair.second);
        ret.insert(loaded);
    }
    return ret;
}

class ResourceManager
{
    // TODO: templates should be one-call-scalable
    FeatureMap features; // template owner
    double scale = 1;
    FeatureMap scaledFeatures;

public:
    ResourceManager(const FeatureDescList &features);
    inline const Feature getImage(const QString &name) {
        FeatureMap *f;
        if (scale == 1) {
            f = &features;
        } else {
            f = &scaledFeatures;
        }
        if (f->find(name) == f->end()) {
            qDebug() << "Requested template not found:" << name;
        }
        return f->at(name);
    }

    void setScale(double scale);
    inline double getScale() {
        return scale;
    }

    // TODO: ideally, this should return an iterable
    const std::list<const Feature*> getTemplates();
};

#endif // RESOURCEMANAGER_H
