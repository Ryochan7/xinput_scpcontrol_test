#ifndef SDLEVENTQUEUE_H
#define SDLEVENTQUEUE_H

#include <QObject>
#include <QTimer>
#include <QSettings>
#include <SDL2/sdl.h>

#include "scpdevice.h"
#include "inputcontroller.h"

class SDLEventQueue : public QObject
{
    Q_OBJECT
public:
    explicit SDLEventQueue(ScpBusDevice *busDevice, QSettings *settings, QObject *parent = 0);
    ~SDLEventQueue();

    void setBusDevice(ScpBusDevice *busDevice);
    InputController* getController();

protected:
    void initializeSDL();
    void quitSDL();

    bool sdlActive;
    InputController controller;
    QTimer testTimer;

signals:
    void closed();

public slots:
    void processQueue();
    void startProcessing();
    void stopProcessing();
    void changePollRate(int pollRate);

};

#endif // SDLEVENTQUEUE_H
