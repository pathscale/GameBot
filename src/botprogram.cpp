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
    QList<QObject*> boxen;

    for (const FeatureMatch &fm : fmlist) {
        const Feature *f = fm.ftr;
        const Sprite *s = fm.sprite;
        const Match &m = fm.match;
        QQmlEngine engine;
        QQmlComponent component(&engine,
                QUrl(QStringLiteral("qrc:/MatchBox.qml")));
        QObject *o = component.create(); // data container compatible with QML that will expose all needed properties
        if (component.status() != QQmlComponent::Ready) {
            qDebug() << "MatchBox didn't load:" << component.status();
            qDebug() << component.errors();
        }
        o->setProperty("x", m.pos.x());
        o->setProperty("y", m.pos.y());
        o->setProperty("width", s->img.size().width);
        o->setProperty("height", s->img.size().height);
        o->setProperty("fit", QString::number(m.value));
        o->setProperty("scale", scale);
        o->setProperty("tiles", f->type.tileWidth);
        o->setProperty("xAnchor", s->anchor.x());
        o->setProperty("yAnchor", s->anchor.y());
        boxen.append(o);
    }

    emit this->matchesChanged(boxen);
    for (const QObject *box : boxen) {
        delete box;
    }
}

void BotProgram::display_heatmap(const std::list<std::pair<QPoint, const Defense*>> &buildings) {
    QList<QObject*> defboxen;
    for (const std::pair<QPoint, const Defense*> bpair : buildings) {
        const QPoint pos = bpair.first;
        const Defense *def = bpair.second;
        QQmlEngine engine;
        QQmlComponent component(&engine, QUrl("qrc:/Defense.qml"));
        QObject *o = component.create();
        if (component.status() != QQmlComponent::Ready) {
            qDebug() << "Component didn't load:" << component.status();
            qDebug() << component.errors();
        }
        o->setProperty("x", pos.x());
        o->setProperty("y", pos.y());
        o->setProperty("range", def->range);
        o->setProperty("dmgValue", def->dps);
        o->setProperty("dmgType", Defense::damageTypeToStr(def->damageType));
        o->setProperty("targets", Defense::targetsToStr(def->targets));
        defboxen.append(o);
    }
    emit this->heatmapChanged(defboxen);
    for (const QObject *box : defboxen) {
        delete box;
    }
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
