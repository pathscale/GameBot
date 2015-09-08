#include "resourcemanager.h"

#include <QFile>
#include <QByteArray>
#include <QDir>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <opencv2/imgcodecs.hpp>

const struct image_with_mask load_cutout(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "Image not present at" << path;
        return {cv::Mat(), cv::Mat()};
    }
    QByteArray bytes = f.readAll();
    const cv::Mat raw_image = cv::imdecode(cv::InputArray(std::vector<char>(bytes.constData(),
                                                          bytes.constData() + bytes.size())),
                                           cv::IMREAD_UNCHANGED);
    cv::Mat rgb;
    cv::Mat alpha;
    int channels = raw_image.channels();
    if (channels == 3) {
        rgb = cv::Mat(raw_image);
    } else if (channels != 4) {
        qDebug() << "Image has uncommon channel count: " << channels << path;
        rgb = cv::Mat(raw_image);
    } else { // 4 channels
        // allocate matrices for split channels
        cv::Size dims = raw_image.size();
        rgb.create(dims, CV_MAKETYPE(raw_image.depth(), 3));
        alpha.create(dims, CV_MAKETYPE(raw_image.depth(), 3));
        cv::Mat out[] = {rgb, alpha};
        // copy subchannels in one call
        // r->r, g->g, b->b, a->Rmask, a->Gmask, a->Bmask
        int from_to[] = {0,0, 1,1, 2,2, 3,3, 3,4, 3,5};
        cv::mixChannels(&raw_image, 1, out, 2, from_to, 6);

        // RGB extracted, check if alpha makes sense
        cv::Mat tmpa;
        cv::extractChannel(raw_image, tmpa, 3);
        if (cv::checkRange(tmpa, true, 0, 255, 255.1)) { // XXX: not very reliable if image isn't UINT8, but unlikely those will be used
            // alpha channel full of 255, erase
            alpha = cv::Mat();
        }
    }
    return {rgb, alpha};
}

const std::list<Sprite> Sprite::alts_from_descs(const SpriteDescAlts &descs) {
    std::list<Sprite> ret;
    for (const SpriteDesc &desc : descs) {
        const struct image_with_mask im = load_cutout(desc.filename);
        ret.push_back(Sprite(im.image, im.mask, desc.anchor, desc.detectionThreshold));
    }
    return ret;
}

const SpriteAlts Sprite::scale_alts(const SpriteAlts &sprites, double scale) {
    SpriteAlts ret;
    cv::Mat scaled;
    cv::Mat scaledMask;
    for (const Sprite &sprite : sprites) {
        const cv::Mat *templ = &sprite.img;
        const cv::Size dims(round(templ->rows * scale), round(templ->cols * scale));
        scaled.create(dims, templ->type());
        cv::resize(*templ, scaled, dims, scale, scale);
        const cv::Mat *outMask;
        if (sprite.mask.empty()) {
            outMask = &sprite.mask;
        } else {
            scaledMask.create(dims, sprite.mask.type());
            cv::resize(sprite.mask, scaledMask, dims, scale, scale);
            outMask = &scaledMask;
        }
        ret.push_back(Sprite(scaled, *outMask, sprite.anchor, sprite.detectionThreshold));
    }
    return ret;
}

Feature::Feature(const SpriteAlts &sprites, const QString humanName, const int maxCount, const int tileWidth, const ObjectBase &type)
    : FeatureBase(humanName, maxCount, tileWidth),
      sprites(sprites),
      type(type)
{}

Feature::Feature(const FeatureDesc &fd)
    : Feature(Sprite::alts_from_descs(fd.sprites), fd.humanName, fd.maxCount, fd.tileWidth, *fd.type)
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
