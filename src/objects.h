#ifndef OBJECTS_H
#define OBJECTS_H
#include <list>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QPoint>
#include <QDebug>

class ObjectBase {
public:
    enum feature_type {
        SCENERY,
        BUILDING,
        DEFENSE,
        STORAGE
    };
    const enum feature_type type;
    const int tileWidth;
protected:
    inline ObjectBase(const enum feature_type ft, const int tileWidth)
        : type(ft),
          tileWidth(tileWidth)
    {}
};

class Scenery : public ObjectBase {
public:
    Scenery(const int tileWidth)
        : ObjectBase(ObjectBase::SCENERY, tileWidth)
    {}
};

class Building : public ObjectBase {
public:
    const int hitpoints;
protected:
    Building(const enum ObjectBase::feature_type ft, const int tileWidth, const int hp)
        : ObjectBase(ft, tileWidth),
          hitpoints(hp)
    {}
public:
    Building(const int tileWidth, const int hp)
        : Building(ObjectBase::BUILDING, tileWidth, hp)
    {}
};

class Storage : public Building {
public:
    enum resource_type {
        GOLD,
        DARK,
        ELIXIR,
    };
    const enum resource_type resource;
    const int amount;
    Storage(const int tileWidth, const int hp, const enum resource_type resource, const int amount)
        : Building(ObjectBase::STORAGE, tileWidth, hp),
          resource(resource),
          amount(amount)
    {}
};

class Defense : public Building {
public:
    enum damage_type {
        SINGLE,
        SPLASH1TILE,
        SPLASH15TILE, // 1.5 tile
        PUSH
    };
    static QString damageTypeToStr(enum damage_type dmg) {
        switch (dmg) {
        case SINGLE: { return "SINGLE"; }
        case SPLASH1TILE: { return "SPLASH1"; }
        case SPLASH15TILE: { return "SPLASH1.5"; }
        case PUSH: { return "PUSH"; }
        default: { return "INVALID"; }
        }
    }

    enum target_type {
        GROUND = 1 << 0,
        AIR = 1 << 1,
    };
    static QString targetsToStr(int targets) {
        QString ret;
        QStringList list;
        if (targets & GROUND) {
            list += "GROUND";
        }
        if (targets & AIR) {
            list += "AIR";
        }
        ret = list.join(",");
        if (ret == "") {
            return "NONE";
        }
        return ret;
    }

    const int range;
    const float dps; // damage per second
    const enum damage_type damageType;
    const unsigned targets;
    Defense(const int tileWidth, const int hp, const int range, const float dps, const enum damage_type damageType, const unsigned targets)
        : Building(ObjectBase::DEFENSE, tileWidth, hp),
          range(range),
          dps(dps),
          damageType(damageType),
          targets(targets)
    {}
};

/* TODO: some buildings need special subclassifiers which can match specific images to special properties:
 * * air sweeper position
 * * storage fill level
 *
 * Benefits:
 * * faster processing (no need to process the whole picture to get details)
 * Requirements:
 * * classifiers need to be rescaled with the rest of images
 * * classifiers are second processing step
 *
 * OR matches need to be converted to object-based classifiers before use.
 */

class SweepValue {
public:
    const unsigned force; // FIXME: what unit?
    const float angle; // radian angle: up=0; +0=left
    SweepValue(unsigned force, unsigned angle)
        : force(force),
          angle(angle)
    {}
};

class DamageValue {
    // dps
    static const std::list<SweepValue> nosweeps;
public:
    const unsigned ground;
    const unsigned air;
    const unsigned total;
    const unsigned splash_ground;
    const unsigned splash_air;
    const unsigned splash_total;
    const std::list<SweepValue> sweeps; // <dps, radian angle: up=0; +0=left>
    DamageValue(unsigned ground, unsigned air, unsigned total,
                unsigned splash_ground, unsigned splash_air, unsigned splash_total,
                const std::list<SweepValue> &sweeps=nosweeps)
        : ground(ground), air(air), total(total),
          splash_ground(splash_ground), splash_air(splash_air), splash_total(splash_total),
          sweeps(sweeps)
    {}
    const QString to_string() const {
        QString ret;
        QTextStream s(&ret);
        if (ground) {
            s << "ground:" << ground << ",";
        }
        if (air) {
            s << "air:" << air << ",";
        }
        if (ground && air) {
            s << "total:" << total << ".";
        }
        if (splash_ground) {
            s << "splash G:" << splash_ground << ",";
        }
        if (splash_air) {
            s << "splash A:" << splash_air << ",";
        }
        if (splash_ground && splash_air) {
            s << "splash total:" << splash_total << ".";
        }

        if (!sweeps.empty()) {
            s << "sweeps: TODO";
        }
        if (ret == "") {
            ret = "0";
        }
        return ret;
    }
};

#endif // OBJECTS_H
