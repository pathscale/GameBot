#include "resourcemanager.h"

#include <QFile>
#include <QByteArray>
#include <QDir>
#include <QDebug>
#include <opencv2/highgui/highgui.hpp>

const cv::Mat load_qfile(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        return cv::Mat();
    }
    QByteArray bytes = f.readAll();
    return cv::imdecode(cv::InputArray(
                            std::vector<char>(bytes.constData(),
                                              bytes.constData() + bytes.size())),
                        1);
}

ResourceManager::ResourceManager(const QString &path)
    : path(path)
{
    // greedy load of all images that can be found - get list from config file later
    const QDir base(path);
    for (const QString s : base.entryList(QDir::Files)) {
        const QString sname = s.split(".").at(0);
        images[sname] = load_qfile(path + "/" + s);
    }
}

