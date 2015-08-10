#ifndef COCBATTLEFIELD_H
#define COCBATTLEFIELD_H

#include <QObject>
#include <QString>
#include <QRect>
#include <QFile>
#include <QDataStream>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class BattlefieldSignals : public QObject
{
    Q_OBJECT
public:
    explicit BattlefieldSignals(QObject *parent = 0);
signals:
    void debugChanged(const QString &filename);
};

inline cv::Mat load_qfile(QString path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        return cv::Mat();
    }
    QDataStream instream(&f);
    std::vector<char> data;
    while (!instream.atEnd()) {
        char chunk[1024];
        int readsize = instream.readRawData(chunk, 1024);
        data.insert(data.end(), &chunk[0], &chunk[readsize]);
    }
    return cv::imdecode(cv::InputArray(data), 1);
}

class CocBattlefield
{
    cv::Mat screen;
    cv::Mat templ = load_qfile(":/cutouts/TH9.png");
    BattlefieldSignals *sig;
public:
    CocBattlefield(const QString &filepath, BattlefieldSignals *proxy=NULL);
    const QRect analyze();
};

#endif // COCBATTLEFIELD_H
