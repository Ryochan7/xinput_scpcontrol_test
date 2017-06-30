#include "scpbuscontainer.h"

static const QString SCP_BUS_CLASS_GUID = "{F679F562-3164-42CE-A4DB-E7DDBE723909}";

ScpBusContainer::ScpBusContainer(QObject *parent) : QObject(parent)
{
    busDevice = 0;
    /*busDevice = new ScpBusDevice (SCP_BUS_CLASS_GUID);
    devtest.open();
    devtest.start();
    devtest.plugin(1);
    */
}

ScpBusContainer::~ScpBusContainer()
{
    if (busDevice)
    {
        delete busDevice;
        busDevice = 0;
    }
}

void ScpBusContainer::initBusDevice()
{
    if (!busDevice)
    {
        busDevice = new ScpBusDevice (SCP_BUS_CLASS_GUID);
        busDevice->open();
        busDevice->start();
        //busDevice.plugin(1);
    }
}

ScpBusDevice* ScpBusContainer::getBusDevice()
{
    return this->busDevice;
}
