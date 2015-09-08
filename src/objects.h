#ifndef OBJECTS_H
#define OBJECTS_H

class ObjectBase {
public:
    enum feature_type {
        SCENERY,
        BUILDING,
        DEFENSE,
        STORAGE
    };
    const enum feature_type type;
    int tileWidth; // TODO: fill in
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

class Storage : public Building {
public:
    enum resource_type {
        GOLD,
        DARK,
        ELIXIR,
    };
    const enum resource_type resource;
    const int amount;
    Storage(const int hp, const enum resource_type resource, const int amount)
        : Building(ObjectBase::STORAGE, 0),
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

    enum target_type {
        GROUND = 1 << 0,
        AIR = 1 << 1,
    };

    const int range;
    const float dps; // damage per second
    const enum damage_type damageType;
    const unsigned targets;
    Defense(const int hp, const int range, const float dps, const enum damage_type damageType, const unsigned targets)
        : Building(ObjectBase::DEFENSE, hp),
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

#endif // OBJECTS_H
