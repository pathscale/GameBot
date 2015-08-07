#ifndef COCBATTLEFIELD_H
#define COCBATTLEFIELD_H

#include <QObject>

class CocBattlefield : public QObject
{
    Q_OBJECT
public:
    explicit CocBattlefield(QObject *parent = 0);

signals:
};

#endif // COCBATTLEFIELD_H
