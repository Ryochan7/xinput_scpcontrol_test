#ifndef UTIL_H
#define UTIL_H

#include <QObject>
#include <QList>
#include <QTimer>
#include <QSettings>

#include "scpdevice.h"
#include "ds4devicetest.h"


class Util : public QObject
{
    Q_OBJECT
public:
    explicit Util(QObject *parent = 0);
    bool discoverDS4Controllers(ScpBusDevice *busDevice, QSettings *settings);
    QList<DS4DeviceTest*>* getControllers();
    DS4DeviceTest* getController();

protected:
    QList<DS4DeviceTest*> controllers;
    QTimer testTimer;

signals:

public slots:
    void changePollRate(int pollRate);
    void stopControllers();

};

#endif // UTIL_H
