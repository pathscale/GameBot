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

typedef std::list<std::pair<const QString, const FeatureDesc>> FeatureList; // TODO: std::pair should be const, but how to do it?

class Template
{
public:
    const QString _path; // for debug only
    cv::Mat img;
    int maxCount; // 0 == inf
    Template(const cv::Mat &img, const int &maxCount, const QString &path="dynamic");
    Template(const FeatureDesc &td);
};

typedef std::map<const QString, Template> TemplateMap; // loaded object

inline TemplateMap load_from_ftrs(const FeatureList &features) {
    // Don't use before Qt resource system is initialized!
    TemplateMap ret;
    // does resource loading by converting FeatureDesc into Template objects (actual loading in constructor)
    for (const std::pair<const QString, const FeatureDesc> ftr_pair : features) {
        const std::pair<const QString, Template> loaded(ftr_pair.first, ftr_pair.second);
        ret.insert(loaded);
    }
    return ret;
}

class ResourceManager
{
    // TODO: templates should be one-call-scalable
    TemplateMap templates; // template owner
public:
    ResourceManager(const FeatureList &features);
    inline const Template getImage(const QString &name) {
        if (templates.find(name) == templates.end()) {
            qDebug() << "Requested template not found:" << name;
        }
        return templates.at(name);
    }
    // TODO: ideally, this should return an iterable
    inline const std::list<const Template*> getTemplates() {
        std::list<const Template*> ret;
        for (const std::pair<const QString, const Template&> &item : templates) {
            ret.push_back(&item.second);
        }
        return ret;
    }
};

#endif // RESOURCEMANAGER_H
