#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QString>
#include <QDebug>
#include <opencv2/imgproc/imgproc.hpp>

class ResourceManager
{
    const QString path;
    std::map<const QString, cv::Mat> images;
public:
    ResourceManager(const QString &path);
    inline const cv::Mat getImage(const QString &rel_filename) {
        if (images.find(rel_filename) == images.end()) {
            qDebug() << "Requested resource not found:" << rel_filename;
        }
        return images[rel_filename];
    }
};

#endif // RESOURCEMANAGER_H
