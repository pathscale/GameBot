#include "resourcemanager.h"

#include <QFile>
#include <QByteArray>
#include <QDir>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <opencv2/highgui/highgui.hpp>

static const cv::Mat load_qfile(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "Image not present at" << path;
        return cv::Mat();
    }
    QByteArray bytes = f.readAll();
    return cv::imdecode(cv::InputArray(
                            std::vector<char>(bytes.constData(),
                                              bytes.constData() + bytes.size())),
                        1);
}

Feature::Feature(const cv::Mat &img, const int maxCount, const float threshold, const QString &path)
    : _path(path),
      img(img),
      maxCount(maxCount),
      detectionThreshold(threshold)
{}

Feature::Feature(const FeatureDesc &fd)
    : _path(fd.filename),
      img(load_qfile(fd.filename)),
      maxCount(fd.maxCount),
      detectionThreshold(fd.detectionThreshold)
{}

ResourceManager::ResourceManager(const FeatureDescList &features)
    : features(load_from_ftrs(features))
{}

void ResourceManager::setScale(double scale) {
    if (scale == this->scale) {
        qDebug() << "redundant scale() call";
    }
    this->scale = scale;
    scaledFeatures.clear();
    for (const std::pair<const QString, const Feature&> &item : features) {
        cv::Mat scaled;
        const cv::Mat *templ = &item.second.img;
        cv::Size s(round(templ->rows * scale), round(templ->cols * scale));
        scaled.create(s, templ->type());
        cv::resize(*templ, scaled, s, scale, scale);
        scaledFeatures.insert(
                    std::pair<const QString, const Feature>(
                        item.first,
                        Feature(scaled,
                                item.second.maxCount,
                                item.second.detectionThreshold,
                                item.second._path + "@" + QString::number(scale))));
    }
}

const std::list<const Feature*> ResourceManager::getTemplates() {
    const FeatureMap *fm;
    if (this->scale == 1) {
        fm = &features;
    } else {
        fm = &scaledFeatures;
    }
    std::list<const Feature*> ret;
    for (const std::pair<const QString, const Feature&> &item : *fm) {
        ret.push_back(&item.second);
    }
    return ret;
}
