#include "cocbattlefield.h"
#include <QDebug>
#include <QTime>
#include <functional>
#include <map>

#include <opencv2/highgui/highgui.hpp>


BattlefieldSignals::BattlefieldSignals(QObject *parent) : QObject(parent)
{}

// image matching

static cv::Mat doMatch(const cv::Mat &img, const MatchTemplate &templ, int match_method, float threshold=0.9)
{
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

static Match FindBestMatch(const cv::Mat &img, const MatchTemplate &templ, const int match_method)
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

static std::list<Match> FindAllMatches(const cv::Mat &img, const MatchTemplate &templ, int match_method, double threshold=0.95)
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

CocBattlefield::~CocBattlefield() {
    if (grid != NULL) {
        delete grid;
    }
}

void CocBattlefield::draw_grid() {
    cv::Mat gridover;
    this->screen.copyTo(gridover);
    // top-left..bottom-right lines
    float topoff = grid->origin.y() / grid->vertDist * grid->horzDist; // x offset between origin and top edge
    float diff = screen.rows / grid->vertDist * grid->horzDist; // x offset between top and bottom edge

    float xstart = grid->origin.x() - topoff;
    float xoff = floor((xstart + diff) / grid->horzDist) * grid->horzDist; // offset to leftomst line on screen
    for (xstart -= xoff;
         xstart < this->screen.cols;
         xstart += grid->horzDist) {
        cv::line(gridover, cv::Point2f(xstart, 0), cv::Point2f(xstart + diff, screen.rows), cv::Scalar(0, 255, 0));
    }
    // top-right..bottom-left lines
    xstart = grid->origin.x() + diff;
    xoff = floor(xstart / grid->horzDist) * grid->horzDist; // offset to leftmost line
    for (xstart -= xoff;
         xstart - diff < screen.cols;
         xstart += grid->horzDist) {
        cv::line(gridover, cv::Point2f(xstart, 0), cv::Point2f(xstart - diff, screen.rows), cv::Scalar(0, 255, 0));
    }
    cv::imwrite("grid.png", gridover);
}

const std::list<FeatureMatch> CocBattlefield::analyze() {
    this->find_loot_numbers();
    this->find_grid();
    this->draw_grid();
    this->buildings->setScale(this->find_scale());
    const std::list<FeatureMatch> ftrs = this->find_buildings();
    find_defenses(ftrs);
    return ftrs;
}

void CocBattlefield::find_defenses(const std::list<FeatureMatch> &buildings) {
    std::list<std::pair<QPoint, const Defense*>> defenses;
    for (const FeatureMatch &fm : buildings) {
        if (fm.ftr->type.type == ObjectBase::DEFENSE) {
            const Defense *def = static_cast<const Defense*>(&fm.ftr->type);
            defenses.push_back(std::pair<QPoint, const Defense*>(fm.match.pos, def));
        }
    }
    this->defense_buildings = defenses;
}

const std::list<FeatureMatch> CocBattlefield::find_buildings() {
    QTime a;
    int i = 0;
    a.start();
    std::list<FeatureMatch> feature_matches;
    for (const Feature *feature : buildings->getTemplates()) {
        int found = 0;
        if (feature->maxCount == 1) {
            for (const Sprite &sprite : feature->sprites) {
                i++;
                const Match m = FindBestMatch(screen, MatchTemplate(sprite.img, sprite.mask), CV_TM_CCORR_NORMED);
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
                const std::list<Match> matches = FindAllMatches(screen, MatchTemplate(sprite.img, sprite.mask), CV_TM_CCORR_NORMED, sprite.detectionThreshold);
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

static float best_match(const cv::Mat &image, const MatchTemplate &templ) {
    return FindBestMatch(image, templ, CV_TM_CCORR_NORMED).value;
}


// XXX: store likely scales (resolution-based) and match against them opportunistically?
static float do_steps(const cv::Mat &image, const MatchTemplate &templ, float minScale, float maxScale, double stepSize = 1.1, float threshold = 0.96, float max_precision = 0.002) { // speed: some stepSize values will be better than others
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
    bool masked = templ.mask.empty();
    cv::Mat scaled;
    cv::Mat scaledMask;
    for (double curScale = minScale;
         curScale <= maxScale; curScale *= stepSize, i++) {
        cv::Size s(round(templ.image.rows * curScale), round(templ.image.cols * curScale));
        scaled.create(s, templ.image.type());
        cv::resize(templ.image, scaled, s, curScale, curScale);

        if (masked) {
            scaledMask.create(s, templ.mask.type());
            cv::resize(templ.mask, scaledMask, s, curScale, curScale);
        }
        const float curValue = best_match(image, MatchTemplate(scaled, scaledMask));
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
double CocBattlefield::find_scale() {
    return 1; // FIXME: scale does more harm than good
    // default prop - TR0, found on most screenshots
    const MatchTemplate probe = buildings->getImage("TR0").sprites.front().get_template();
    return do_steps(screen, probe, 0.5, 2.0);
}

const Match try_corner(const cv::Mat &binary, const QString &t) {
    const struct image_with_mask templ = load_cutout(t);
    /*cv::Mat templ_gray;
    cv::extractChannel(templ.image, templ_gray, 0);
    cv::Mat mask;
    templ.image.copyTo(mask);
    cv::dilate(mask, mask, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2)), cv::Point(-1, -1), 3);*/
    return FindBestMatch(binary, MatchTemplate(templ.image, cv::Mat()), CV_TM_CCORR_NORMED);
}

void CocBattlefield::find_grid() {
    /* Detects the red drop zone outline and setups grid against it.
     * Outline extraction: threshold(red-green) (keeps orange)
     *   Better: inrange(reddish color)
     * Grid calibration: find corners
     *   left half of image - find left corner (anchor just outside tip)
     *   right half - right corner
     *   Better: find lines in both halves (template?)
     *           rotate
     *           sum by rows/columns
     *           threshold sums
     *           find distances between sums & offset
     *           calculate grid
     *   Also: opencv find_lines? & calculate grid
     */
    cv::Mat red;
    cv::Mat green;
    cv::extractChannel(screen, green, 1);
    cv::extractChannel(screen, red, 2);
    cv::Mat rmg(screen.size(), CV_16U);
    cv::subtract(red, green, rmg);
    cv::Mat &binary = red; // red, green unneeded - reuse
    cv::threshold(rmg, binary, 50, 255, cv::THRESH_BINARY);
    //cv::imwrite("rmg.png", rmg);
    //cv::imwrite("binary.png", binary);
    // hardcoded image & offset
    // stupidest possible detection of polygon size: split image vertically. left side will have inside on the right, right side will have inside on the left. Important because grid intersections are always on the outside.
    QPoint templ_offset;
    Match *m = NULL;
    // left part
    int halfwidth = binary.cols / 2;
    cv::Mat roi = binary(cv::Rect2i(0, 0, halfwidth, binary.rows));
    Match ml = try_corner(roi, ":/cutouts/grid/left.png");
    qDebug() << ml.pos << ml.value;
    m = new Match(ml);
    if (ml.value > 0.7) {
        templ_offset = QPoint(3, 28);
    } else {
        delete m;
        roi = binary(cv::Rect2i(halfwidth, 0, binary.cols - halfwidth, binary.rows));
        Match mr = try_corner(roi, ":/cutouts/grid/right.png");
        qDebug() << mr.value;
        m = new Match(mr);
        if (mr.value > 0.7) {
            templ_offset = QPoint(halfwidth + 37, 31);
        } else {
            qDebug() << "Grid detection inadequate";
        }
    }
    qDebug() << "Grid position certainty" << m->value;
    this->grid = new Grid(m->pos + templ_offset);
    delete m;
}

static int number_match(const cv::Mat& area) {
    QString templ_path = ":/cutouts/ui/font/#.png";
    std::map<int, int> number_map;

    for (int digit = 0; digit <= 9; ++digit) {
        templ_path[18] = '0' + digit;
        struct image_with_mask font_templ = load_cutout(templ_path);
        cv::Mat font_grayscale;
        cv::cvtColor(font_templ.image, font_grayscale, cv::COLOR_BGR2GRAY);

        for (Match m : FindAllMatches(area, MatchTemplate(font_grayscale, cv::Mat()), CV_TM_CCORR_NORMED)) {
            number_map[m.pos.x()] = digit;
        }
    }

    // FIXME: verify that we didn't miss any digits
    int ret = 0;
    for (std::pair<int, int> x : number_map) {
        ret *= 10;
        ret += x.second;
    }

    return ret;
}

static void find_resource(const cv::Mat& scan_area, const cv::Mat& area, const QString& cutout_path, float threshold, int* result) {
    struct image_with_mask res_templ = load_cutout(cutout_path);

	for (float scale : {1.0, 0.85}) {
		cv::Mat templ_image;
		cv::resize(res_templ.image, templ_image, cv::Size(0, 0), scale, scale);

		Match m = FindBestMatch(scan_area, MatchTemplate(templ_image, cv::Mat()), CV_TM_CCORR_NORMED);
		if (m.value >= threshold) {
			qDebug() << "Found" << cutout_path << "at" << m.pos << "scale" << scale;

			// text should be no higher than the icon,
			// and no wider than quarter of screen width
			cv::Mat text_area = area(cv::Rect(m.pos.x() + res_templ.image.cols, m.pos.y(), 0.25 * area.cols, res_templ.image.rows));
			*result = number_match(text_area);
			qDebug() << "Text matching result:" << *result;
			break;
		}
	}
}

void CocBattlefield::find_loot_numbers(float threshold) {
    cv::Mat grayscale;
    cv::cvtColor(screen, grayscale, cv::COLOR_BGR2GRAY);

    cv::Mat scan_area = screen(cv::Rect(0, 0, 0.05 * grayscale.cols, grayscale.rows));

    find_resource(scan_area, grayscale, ":/cutouts/ui/gold.png", threshold, &available_loot.gold);
    find_resource(scan_area, grayscale, ":/cutouts/ui/elixir.png", threshold, &available_loot.elixir);
    find_resource(scan_area, grayscale, ":/cutouts/ui/darkelixir.png", threshold, &available_loot.dark_elixir);
}
