#ifndef OBJECTS_H
#define OBJECTS_H


class ObjectBase {
public:
    enum feature_type {
        SCENERY,
        BUILDING,
        DEFENSE,
    };
    const enum feature_type type;
protected:
    inline ObjectBase(const enum feature_type ft)
        : type(ft)
    {}
};

class Scenery : public ObjectBase {
public:
    Scenery()
        : ObjectBase(ObjectBase::SCENERY)
    {}
};

class Building : public ObjectBase {
public:
    const int hitpoints;
protected:
    Building(const enum ObjectBase::feature_type ft, const int hp)
        : ObjectBase(ft),
          hitpoints(hp)
    {}
public:
    Building(const int hp)
        : Building(ObjectBase::BUILDING, hp)
    {}
};

class Defense : public Building {
public:
    enum damage_type {
        SINGLE,
        SPLASH1TILE
    };

    enum target_type {
        GROUND = 1 << 0,
        AIR = 1 << 1,
    };

    const int range;
    const float dps; // damage per second
    const enum damage_type damageType;
    const enum target_type targets;
    Defense(const int hp, const int range, const float dps, const enum damage_type damageType, const enum target_type targets)
        : Building(ObjectBase::DEFENSE, hp),
          range(range),
          dps(dps),
          damageType(damageType),
          targets(targets)
    {}
};

#endif // OBJECTS_H