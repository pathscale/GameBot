#include "cocbattlefield.h"
#include <opencv2/highgui/highgui.hpp>
#include <QDebug>

CocBattlefield::CocBattlefield(const QString &filepath)
    : screen(cv::imread(filepath.toStdString()))
{
    if (screen.data == NULL) {
        qDebug() << "abandon ship, the image didn't load (FIXME: catch gracefully)";
    } else {
        qDebug() << "loaded image Ok";
    }
}

const QRect CocBattlefield::analyze() {
    return QRect(10, 10, 100, 100);
}
