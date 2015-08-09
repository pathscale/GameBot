#include "cocbattlefield.h"
#include <QDebug>

BattlefieldSignals::BattlefieldSignals(QObject *parent) : QObject(parent)
{}

// image matching
cv::Mat doMatch(cv::Mat img, cv::Mat templ, int match_method)
{
    cv::Mat result;

    // Create the result matrix
    int result_cols =  img.cols - templ.cols + 1;
    int result_rows = img.rows - templ.rows + 1;

    result.create( result_cols, result_rows, CV_32FC1 );

    // Do the Matching and Normalize
    //  Method = 0: CV_TM_SQDIFF, 1: CV_TM_SQDIFF_NORMED 2: CV_TM_CCORR 3: CV_TM_CCORR_NORMED 4: CV_TM_CCOEFF 5: CV_TM_CCOEFF_NORMED
    //match_method = CV_TM_CCOEFF_NORMED;
    cv::matchTemplate(img, templ, result, match_method );
    //cv::threshold(result, result, 0.9, 1.0, CV_THRESH_TOZERO);
    //normalize( result, result, 1, 100, NORM_MINMAX, -1, Mat() );

    return result;
}

CocBattlefield::CocBattlefield(const QString &filepath, BattlefieldSignals *proxy)
    : screen(cv::imread(filepath.toStdString())),
      sig(proxy)
{
    if ((screen.data == NULL) || (templ.data == NULL)) {
        qDebug() << "abandon ship, some images didn't load (FIXME: catch gracefully)";
    } else {
        qDebug() << "loaded images Ok";
    }
}

const QRect CocBattlefield::analyze() {
    cv::imwrite("detect.png", doMatch(screen, templ, CV_TM_CCORR_NORMED) * 256);
    emit sig->debugChanged("detect.png");
    return QRect(10, 10, 100, 100);
}
