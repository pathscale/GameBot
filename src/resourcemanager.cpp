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

Template::Template(const cv::Mat &img, const int &maxCount, const QString &path)
    : _path(path),
      img(img),
      maxCount(maxCount)
{}

Template::Template(const FeatureDesc &td)
    : _path(td.filename),
      img(load_qfile(td.filename)),
      maxCount(td.maxCount)
{}

ResourceManager::ResourceManager(const FeatureList &features)
    : templates(load_from_ftrs(features))
{}
