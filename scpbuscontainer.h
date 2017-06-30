#ifndef SCPBUSCONTAINER_H
#define SCPBUSCONTAINER_H

#include <QObject>

#include "scpdevice.h"


class ScpBusContainer : public QObject
{
    Q_OBJECT
public:
    explicit ScpBusContainer(QObject *parent = 0);
    ~ScpBusContainer();

    ScpBusDevice* getBusDevice();

protected:
    ScpBusDevice *busDevice;

signals:

public slots:
    void initBusDevice();
};

#endif // SCPBUSCONTAINER_H
