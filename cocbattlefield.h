#ifndef COCBATTLEFIELD_H
#define COCBATTLEFIELD_H

#include <QString>
#include <QRect>

#include <opencv2/imgproc/imgproc.hpp>

class CocBattlefield
{
    cv::Mat screen;
public:
    CocBattlefield(const QString &filepath);
    const QRect analyze();
};

#endif // COCBATTLEFIELD_H
