#include "cocbattlefield.h"
#include <QDebug>

BattlefieldSignals::BattlefieldSignals(QObject *parent) : QObject(parent)
{}

// image matching

cv::Mat doMatch(cv::Mat img, cv::Mat templ, int match_method, float threshold=0.9)
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
    cv::threshold(result, result, 0.9, 1.0, CV_THRESH_TOZERO);
    //normalize( result, result, 1, 100, NORM_MINMAX, -1, Mat() );

    return result;
}

class Match {
public:
    QPoint pos;
    float value;
    Match(int x, int y, float value)
        : pos(x, y), value(value)
    {}
};

Match FindBestMatch(const cv::Mat &img, const cv::Mat &templ, const int match_method)
{
    cv::Mat result = doMatch(img, templ, match_method);

    /*// Debug
    char* image_window = "Source Image";
    namedWindow( image_window, CV_WINDOW_AUTOSIZE );
    Mat img_display;
    img.copyTo( img_display );
    */

    // Localize the best match with minMaxLoc
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

    // For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    cv::Point matchLoc = (match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED) ? minLoc : maxLoc;
    double matchVal = (match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED) ? minVal : maxVal;
    //int x = matchLoc.x + templ.cols/2; // for midpoint
    //int y = matchLoc.y + templ.rows/2;
    int x = matchLoc.x;
    int y = matchLoc.y;

    /*// Debug
    rectangle( img_display, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar(0,0xff,0), 2, 8, 0 );
    imshow( image_window, img_display );
    waitKey(0);
    */
    return Match(x, y, matchVal);
}

std::list<Match> FindAllMatches(const cv::Mat &img, const cv::Mat &templ, int match_method, double threshold=0.9)
{
    cv::Mat result = doMatch(img, templ, match_method);

    std::list<Match> matches;

    for(int i = 0; i < 100; i++) // end condition is the threshold check; 100 in case of stupid threshold
    {
        double minVal, maxVal;
        cv::Point minLoc, maxLoc;
        cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

        // For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
        double matchVal = (match_method==CV_TM_SQDIFF || match_method==CV_TM_SQDIFF_NORMED) ? minVal : maxVal;

        if (((match_method==CV_TM_SQDIFF || match_method==CV_TM_SQDIFF_NORMED) && matchVal > threshold) ||
             (match_method!=CV_TM_SQDIFF && match_method!=CV_TM_SQDIFF_NORMED && matchVal < threshold) ) {
            break;
        }

        cv::Point matchLoc = (match_method==CV_TM_SQDIFF || match_method==CV_TM_SQDIFF_NORMED) ? minLoc : maxLoc;

        // FIXME: maybe useful but doesn't belong here. Fill haystack with pure green so we don't match this same location
        //cv::rectangle(img, matchLoc, cv::Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows),
          //              CV_RGB(0,255,0), 2);

        // Fill results array with hi or lo vals, so we don't match this same location
        cv::Scalar fillVal = (match_method==CV_TM_SQDIFF || match_method==CV_TM_SQDIFF_NORMED) ? 1 : 0;
        cv::floodFill(result, matchLoc, fillVal, 0, cv::Scalar(0.1), cv::Scalar(1.0));

        // Add matched location to the vector
        //matches[count].x = matchLoc.x + templ.cols/2; // for midpoint
        //matches[count].y = matchLoc.y + templ.rows/2;
        matches.push_back(Match(matchLoc.x, matchLoc.y, matchVal));
    }
    return matches;
}

CocBattlefield::CocBattlefield(const QString &filepath, BattlefieldSignals *proxy)
    : screen(cv::imread(filepath.toStdString())),
      sig(proxy)
{
    if ((screen.data == NULL) || (townHall.data == NULL)) {
        qDebug() << "abandon ship, some images didn't load (FIXME: catch gracefully)";
    } else {
        qDebug() << "loaded images Ok";
    }
}

const QVariantList CocBattlefield::analyze() {
    Match townMatch = FindBestMatch(screen, townHall, CV_TM_CCORR_NORMED);
    std::list<Match> defenseMatches = FindAllMatches(screen, fw, CV_TM_CCORR_NORMED);
    // TODO: get rid of Qt containers in logic code and move a generic building structure
    QVariantList boxen;
    if (townMatch.value == 0) {
        qDebug() << "Town hall not found";
    } else {
        QVariant tbox(QRect(townMatch.pos, QSize(townHall.size().width, townHall.size().height)));
        boxen.append(tbox);
    }
    for (Match m : defenseMatches) {
        qDebug() << "match" << m.pos << "certain" << m.value;
        QVariant mbox(QRect(m.pos, QSize(fw.size().width, fw.size().height)));
        boxen.append(mbox);
    }
    return boxen;
}
