#ifndef COCBATTLEFIELD_H
#define COCBATTLEFIELD_H

#include <QObject>
#include <QString>
#include <QRect>

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

class CocBattlefield
{
    cv::Mat screen;
    cv::Mat templ = cv::imread("../levelme_coc/images/0x4_small.png");
    BattlefieldSignals *sig;
public:
    CocBattlefield(const QString &filepath, BattlefieldSignals *proxy=NULL);
    const QRect analyze();
};

#endif // COCBATTLEFIELD_H
