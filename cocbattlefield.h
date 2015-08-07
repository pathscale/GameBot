#ifndef COCBATTLEFIELD_H
#define COCBATTLEFIELD_H

#include <QString>
#include <QRect>

class CocBattlefield
{
public:
    CocBattlefield(const QString &filepath);
    const QRect analyze();
};

#endif // COCBATTLEFIELD_H
