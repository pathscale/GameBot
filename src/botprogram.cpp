#include "botprogram.h"
#include <functional>

#include <QQmlEngine>
#include <QQmlComponent>
#include <QDebug>
BotProgram::BotProgram(QObject *parent)
    : QObject(parent),
      sigs(this)
{
    connect(&sigs, SIGNAL(imageChanged(QString)), this, SLOT(onImageChanged(QString)));
}

void BotProgram::onImageChanged(const QString &filename) {
    emit imageChanged(QUrl::fromLocalFile(filename));
}

void BotProgram::display_matches(const std::list<FeatureMatch> &fmlist) {
    const double scale = this->battlefield->getScale();
    QVariantList boxen;
    for (const FeatureMatch &fm : fmlist) {
        const Feature *f = fm.ftr;
        const Sprite *s = fm.sprite;
        const Match &m = fm.match;
        QVariantMap box = {{"x", m.pos.x()},
                           {"y", m.pos.y()},
                           {"width", s->img.size().width},
                           {"height", s->img.size().height},
                           {"fit", QString::number(m.value)},
                           {"scale", scale},
                           {"tiles", f->type.tileWidth},
                           {"xAnchor", s->anchor.x()},
                           {"yAnchor", s->anchor.y()}};
        boxen.append(box);
    }
    emit this->matchesChanged(boxen);
}

void BotProgram::display_heatmap(const std::list<std::pair<QPoint, const Defense*>> &buildings) {
    QVariantList defboxen;
    for (const std::pair<QPoint, const Defense*> bpair : buildings) {
        const QPoint pos = bpair.first;
        const Defense *def = bpair.second;
        QVariantMap o = {{"x", pos.x()},
                         {"y", pos.y()},
                         {"range", def->range},
                         {"dmgValue", def->dps},
                         {"dmgType", Defense::damageTypeToStr(def->damageType)},
                         {"targets", Defense::targetsToStr(def->targets)}};
        defboxen.append(o);
    }
    emit this->heatmapChanged(defboxen);
}

void BotProgram::loadUrl(const QUrl &url) {
    this->battlefield = new CocBattlefield(url.toLocalFile(), &buildings, &sigs);
    const std::list<FeatureMatch>f = this->battlefield->analyze();
    this->display_matches(f);
    this->display_heatmap(this->battlefield->get_defense_buildings());
}

QString BotProgram::getDamageText(int x, int y) {
//    QPointF p = this->battlefield->screen_to_gridF(QPoint(x, y));
//    return QString::number(p.x()) + "," + QString::number(p.y());
    return this->battlefield->get_pixel_damage(QPoint(x, y)).to_string();
}

BotProgram::~BotProgram() {
    if (this->battlefield) {
        delete this->battlefield;
    }
}
