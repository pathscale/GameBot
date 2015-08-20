#include "cocbattlefield.h"
#include <QDebug>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>

#include <functional>

#include <opencv2/highgui/highgui.hpp>

BattlefieldSignals::BattlefieldSignals(QObject *parent) : QObject(parent)
{}

// image matching

static cv::Mat doMatch(cv::Mat img, cv::Mat templ, int match_method, float threshold=0.9)
{
    cv::Mat result;

    // Create the result matrix
    int result_cols =  img.cols - templ.cols + 1;
    int result_rows = img.rows - templ.rows + 1;

    result.create( result_cols, result_rows, CV_32FC1 );

    // Do the Matching and Normalize
    //  Method = 0: CV_TM_SQDIFF, 1: CV_TM_SQDIFF_NORMED 2: CV_TM_CCORR 3: CV_TM_CCORR_NORMED 4: CV_TM_CCOEFF 5: CV_TM_CCOEFF_NORMED
    cv::matchTemplate(img, templ, result, match_method);
    if (threshold != 0) {
        cv::threshold(result, result, threshold, 1.0, CV_THRESH_TOZERO);
    }

    return result;
}

static Match FindBestMatch(const cv::Mat &img, const cv::Mat &templ, const int match_method)
{
    cv::Mat result = doMatch(img, templ, match_method);

    // Localize the best match with minMaxLoc
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

    // For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    cv::Point matchLoc = (match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED) ? minLoc : maxLoc;
    double matchVal = (match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED) ? minVal : maxVal;
    int x = matchLoc.x;
    int y = matchLoc.y;

    /*// Debug
    rectangle( img_display, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar(0,0xff,0), 2, 8, 0 );
    imshow( image_window, img_display );
    waitKey(0);
    */
    return Match(x, y, matchVal);
}

static std::list<Match> FindAllMatches(const cv::Mat &img, const cv::Mat &templ, int match_method, double threshold=0.95)
{
    cv::Mat result = doMatch(img, templ, match_method); // FIXME for performance - make method of some object with lifetime linked to BattleField, overallocate and reuse

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

CocBattlefield::CocBattlefield(const QString &filepath, ResourceManager *buildings, BattlefieldSignals *proxy)
    : screen(cv::imread(filepath.toStdString())),
      buildings(buildings),
      sig(proxy)
{
    if (screen.data == NULL) {
        qDebug() << "abandon ship, screen image didn't load (FIXME: catch gracefully)";
    } else {
        qDebug() << "loaded images Ok";
    }
}

const std::list<FeatureMatch> CocBattlefield::analyze() {
    this->buildings->setScale(this->find_scale());
    std::list<FeatureMatch> feature_matches;
    for (const Feature *feature : buildings->getTemplates()) {
        if (feature->maxCount == 1) {
            const Match m = FindBestMatch(screen, feature->img, CV_TM_CCORR_NORMED);
            if (m.value > 0) {
                feature_matches.push_back(FeatureMatch(feature, m));
            } else {
                qDebug() << "Not found" << feature->_path;
            }
        } else {
            // FIXME: limited maxCount
            const std::list<Match> matches = FindAllMatches(screen, feature->img, CV_TM_CCORR_NORMED);
            qDebug() << "Found" << matches.size() << "of" << feature->_path;
            for (const Match &m : matches) {
                feature_matches.push_back(FeatureMatch(feature, m));
            }
        }
    }
    return feature_matches;
}

static float best_match(const cv::Mat &image, const cv::Mat &templ) {
    return FindBestMatch(image, templ, CV_TM_CCORR_NORMED).value;
}


// XXX: store likely scales (resolution-based) and match against them opportunistically?
static float do_steps(const cv::Mat &image, const cv::Mat &templ, float minScale, float maxScale, double stepSize = 1.1, float threshold = 0.96, float max_precision = 0.002) { // speed: some stepSize values will be better than others
    qDebug() << "steps" << minScale << "to" << maxScale << "by" << stepSize;
    if (maxScale - minScale < max_precision || stepSize < max_precision + 1) { // recursion stop condition
        qDebug() << "Scale not found";
        return 0;
    }

    struct find_res {
        double scale;
        float value;
    };

    std::vector<struct find_res> results; // matching may give local maxima, they need to be ignored. Ideally, they should be investigated based on some threshold difference
    float maxValue = 0;
    unsigned int maxIdx = 0;
    unsigned int i = 0;
    cv::Mat scaled;
    for (double curScale = minScale;
         curScale <= maxScale; curScale *= stepSize, i++) {
        cv::Size s(round(templ.rows * curScale), round(templ.cols * curScale));
        scaled.create(s, templ.type());
        cv::resize(templ, scaled, s, curScale, curScale);

        const float curValue = best_match(image, scaled);
        qDebug() << "@" << curScale << curValue;
        results.push_back({curScale, curValue});
        if (curValue > maxValue) {
            maxValue = curValue;
            maxIdx = i;
        }
    }

    struct find_res result = results.at(maxIdx);
    if (result.value > threshold) {
        qDebug() << "found scale with accuracy" << result.value;
        return result.scale;
    }
    if (results.size() == 1) {
        return result.scale;
    }

    // decide whether to dive into the interval in smaller or higher scales
    minScale = result.scale; // safety

    // smaller allowed - assume it's the case
    float minValue = 0;
    if (maxIdx > 0) {
        minScale = results.at(maxIdx - 1).scale;
        minValue = results.at(maxIdx - 1).value;
    }
    maxScale = result.scale;

    if (maxIdx < results.size() - 1 && results.at(maxIdx + 1).value > minValue) {
        // larger preferred, take over
        minScale = result.scale;
        maxScale = results.at(maxIdx + 1).scale;
    }

    return do_steps(image, templ, minScale, maxScale, pow(stepSize, 1./7), threshold, max_precision);
}

// find scale relative to images in owned buildings list
float CocBattlefield::find_scale() {
    // default prop - TR0, found on most screenshots
    const cv::Mat probe = buildings->getImage("TR0").img;
    return do_steps(screen, probe, 0.5, 2.0);
}
