#ifndef DETECTOR_H
#define DETECTOR_H

#include <QTime>
#include <QPoint>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "resourcemanager.h"

class Match {
public:
    const QPoint pos;
    const float value; // if methods other than TM_CCORR_NORMED are ever used, value should be normalized to reflect accuracy and not raw match
    Match(const int x, const int y, const float value)
        : pos(x, y), value(value)
    {}
    Match(const Match &m)
        : pos(m.pos),
          value(m.value)
    {}
};

class FeatureMatch {
public:
    const Feature *ftr;
    const Sprite *sprite;
    const Match match;
    FeatureMatch(const Feature *ftr, const Sprite *sprite, const Match &match)
        : ftr(ftr),
          sprite(sprite),
          match(match)
    {}
};


// image matching

cv::Mat doMatch(const cv::Mat &img, const MatchTemplate &templ, int match_method, float threshold=0.9);

const Match FindBestMatch(const cv::Mat &img, const MatchTemplate &templ, const int match_method);

const std::list<Match> FindAllMatches(const cv::Mat &img, const MatchTemplate &templ, int match_method, double threshold=0.95);


const std::list<FeatureMatch> match_features(const cv::Mat &image, const std::list<const Feature *> &features);

class Detector
{
public:
    Detector();
};

#endif // DETECTOR_H
