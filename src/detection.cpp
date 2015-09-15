#include "detection.h"

cv::Mat doMatch(const cv::Mat &img, const MatchTemplate &templ, int match_method, float threshold) {
    cv::Mat result;
    // Create the result matrix
    int result_cols =  img.cols - templ.image.cols + 1;
    int result_rows = img.rows - templ.image.rows + 1;

    result.create( result_cols, result_rows, CV_32FC1 );

    // Do the Matching and Normalize
    //  Method = 0: CV_TM_SQDIFF, 1: CV_TM_SQDIFF_NORMED 2: CV_TM_CCORR 3: CV_TM_CCORR_NORMED 4: CV_TM_CCOEFF 5: CV_TM_CCOEFF_NORMED
    cv::matchTemplate(img, templ.image, result, match_method, templ.mask);
    if (threshold != 0) {
        cv::threshold(result, result, threshold, 1.0, CV_THRESH_TOZERO);
    }

    return result;
}

const Match FindBestMatch(const cv::Mat &img, const MatchTemplate &templ, const int match_method)
{
    const cv::Mat result = doMatch(img, templ, match_method, 0);
    // Localize the best match with minMaxLoc
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

    // For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    cv::Point matchLoc = (match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED) ? minLoc : maxLoc;
    double matchVal = (match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED) ? minVal : maxVal;
    int x = matchLoc.x;
    int y = matchLoc.y;
/*
cv::Mat chan;
img.copyTo(chan);
cv::Mat sq(img.rows, img.cols, img.type(), 0.0);
cv::Mat sub = sq(cv::Rect2i(x, y, templ.image.cols, templ.image.rows));
//cv::rectangle(sub, cv::Rect2i(0, 0, 10, 10), cv::Scalar(255));
templ.image.copyTo(sub);
cv::Mat in[] = {sq, chan, chan};
cv::Mat out;
cv::merge(in, 3, out);
cv::imwrite("matchbox.png", out);*/
    return Match(x, y, matchVal);
}

const std::list<Match> FindAllMatches(const cv::Mat &img, const MatchTemplate &templ, int match_method, double threshold)
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

const std::list<FeatureMatch> match_features(const cv::Mat &image, const std::list<const Feature*> &features) {
    QTime a;
    int i = 0;
    a.start();
    std::list<FeatureMatch> feature_matches;
    for (const Feature *feature : features) {
        int found = 0;
        if (feature->maxCount == 1) {
            for (const Sprite &sprite : feature->sprites) {
                i++;
                const Match m = FindBestMatch(image, MatchTemplate(sprite.img, sprite.mask), CV_TM_CCORR_NORMED);
                if (m.value > 0) {
                    feature_matches.push_back(FeatureMatch(feature, &sprite, m));
                    found++;
                }
            }
            if (!found) {
                qDebug() << "Not found" << feature->humanName;
            }
        } else {
            int found = 0;
            for (const Sprite &sprite : feature->sprites) {
                // FIXME: limited maxCount
                i++;
                const std::list<Match> matches = FindAllMatches(image, MatchTemplate(sprite.img, sprite.mask), CV_TM_CCORR_NORMED, sprite.detectionThreshold);
                for (const Match &m : matches) {
                    feature_matches.push_back(FeatureMatch(feature, &sprite, m));
                }
                found += matches.size();
            }
            qDebug() << "Found" << found << "of" << feature->humanName;
        }
    }
    qDebug() << "elapsed" << a.elapsed() << "per search" << a.elapsed() / i;
    return feature_matches;
}




Detector::Detector()
{

}

