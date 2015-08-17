#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QString>
#include <QDebug>
#include <opencv2/imgproc/imgproc.hpp>

/* TemplData != Template because:
 * 1. data must be immutable
 * 2. loading images must be done after Qt resource system initializes
 * 3. if lazy loading is used, it will cut into time allocated for processing - unacceptable
 */

class FeatureDesc {
public:
    const QString filename;
    const int maxCount;
    inline FeatureDesc(const QString &filename, const int &maxCount)
        : filename(filename),
          maxCount(maxCount)
    {}
};

typedef std::list<std::pair<const QString, const FeatureDesc>> FeatureDescList; // TODO: std::pair should be const, but how to do it?

class Feature
{
public:
    const QString _path; // for debug only
    cv::Mat img;
    int maxCount; // 0 == inf
    Feature(const cv::Mat &img, const int &maxCount, const QString &path="dynamic");
    Feature(const FeatureDesc &td);
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
        if (features.find(name) == features.end()) {
            qDebug() << "Requested template not found:" << name;
        }
        return features.at(name);
    }

    void setScale(double scale);

    // TODO: ideally, this should return an iterable
    const std::list<const Feature*> getTemplates();
};

#endif // RESOURCEMANAGER_H
