#ifndef SDLEVENTQUEUE_H
#define SDLEVENTQUEUE_H

#include <QObject>
#include <QTimer>
#include <SDL2/sdl.h>

#include "scpdevice.h"
#include "inputcontroller.h"

class SDLEventQueue : public QObject
{
    Q_OBJECT
public:
    explicit SDLEventQueue(ScpBusDevice *busDevice, QObject *parent = 0);
    explicit SDLEventQueue(QObject *parent = 0);
    ~SDLEventQueue();

    void setBusDevice(ScpBusDevice *busDevice);

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

};

#endif // SDLEVENTQUEUE_H
