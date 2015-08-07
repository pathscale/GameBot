#ifndef BOTPROGRAM_H
#define BOTPROGRAM_H

#include <QObject>
#include <QUrl>

class BotProgram : public QObject
{
    Q_OBJECT
    QString url;
public:
    explicit BotProgram(QObject *parent = 0);
    Q_INVOKABLE void loadUrl(QUrl);
signals:

public slots:
};

#endif // BOTPROGRAM_H
