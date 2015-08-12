#include "resourcemanager.h"

#include <QFile>
#include <QByteArray>
#include <QDir>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <opencv2/highgui/highgui.hpp>

const cv::Mat load_qfile(const QString &path) {
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

Template::Template(const cv::Mat &img, const int &maxCount)
    : img(img),
      maxCount(maxCount)
{}

Template::Template(const FeatureDesc &td)
    : img(load_qfile(td.filename)),
      maxCount(td.maxCount)
{}


const QString basePath(const QString &fullPath) {
    for (int i = fullPath.size() - 1; i > 0; i--) {
        if (fullPath[i] == QChar('/')) {
            QString ret(fullPath);
            ret.truncate(i);
            return ret;
        }
    }
    return "";
}

ResourceManager::ResourceManager(const FeatureList &features)
    : templates(load_from_ftrs(features))
{}
