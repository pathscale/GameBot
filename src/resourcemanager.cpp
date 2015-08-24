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

const std::list<Sprite> Sprite::alts_from_descs(const SpriteDescAlts &descs) {
    std::list<Sprite> ret;
    for (const SpriteDesc &desc : descs) {
        ret.push_back(Sprite(load_qfile(desc.filename), desc.anchor, desc.detectionThreshold));
    }
    return ret;
}

const SpriteAlts Sprite::scale_alts(const SpriteAlts &sprites, double scale) {
    SpriteAlts ret;
    cv::Mat scaled;
    for (const Sprite &sprite : sprites) {
        const cv::Mat *templ = &sprite.img;
        cv::Size s(round(templ->rows * scale), round(templ->cols * scale));
        scaled.create(s, templ->type());
        cv::resize(*templ, scaled, s, scale, scale);
        ret.push_back(Sprite(*templ, sprite.anchor, sprite.detectionThreshold));
    }
    return ret;
}

Feature::Feature(const SpriteAlts &sprites, const QString humanName, const int maxCount, const int tileWidth, const ObjectBase &type)
    : FeatureBase(humanName, maxCount, tileWidth, type),
      sprites(sprites)
{}

Feature::Feature(const FeatureDesc &fd)
    : Feature(Sprite::alts_from_descs(fd.sprites), fd.humanName, fd.maxCount, fd.tileWidth, fd.type)
{}

Feature Feature::scaled(double scale) const {
    return Feature(Sprite::scale_alts(sprites, scale),
                   humanName,
                   maxCount,
                   tileWidth,
                   type);
}

ResourceManager::ResourceManager(const FeatureDescList &features)
    : features(load_from_ftrs(features))
{}

void ResourceManager::setScale(double scale) {
    if (scale == this->scale) {
        qDebug() << "redundant scale() call (" << scale << ")";
    }
    this->scale = scale;
    if (scale == 1) {
        return;
    }
    scaledFeatures.clear();
    for (const std::pair<const QString, const Feature&> &item : features) {
        scaledFeatures.insert(
                    std::pair<const QString, const Feature>(
                        item.first,
                        item.second.scaled(scale)));
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
