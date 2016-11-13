#include <QDebug>
#include <QTimer>
#include <QMap>

#include "defaultsettings.h"
#include "sdleventqueue.h"

static QMap<int, SDL_Joystick*> sdlJoysticks;
static QMap<SDL_JoystickID, SDL_Joystick*> joystickMap;

SDLEventQueue::SDLEventQueue(ScpBusDevice *busDevice, QSettings *settings, QObject *parent) :
    QObject(parent),
    controller(busDevice, this)
{
    testTimer.setParent(this);
    //testTimer.setInterval(10);
    testTimer.setInterval(settings->value("pollRate", ProgramDefaults::pollRate).toInt());
    testTimer.setSingleShot(false);
    testTimer.setTimerType(Qt::PreciseTimer);
    sdlActive = false;

    initializeSDL();
    for (int i=0; i < SDL_NumJoysticks(); i++)
    {
        SDL_Joystick *current = SDL_JoystickOpen(i);
        sdlJoysticks.insert(i, current);
        QString tempname = QString::fromLocal8Bit(SDL_JoystickName(current));
        //if (tempname == QStringLiteral("Twin USB Joystick"))
        if (tempname == QStringLiteral("MP-8866 Dual USB Joypad"))
        {
            SDL_JoystickID tempID = SDL_JoystickInstanceID(current);
            if (!joystickMap.contains(tempID))
            {
                joystickMap.insert(tempID, current);
            }
        }
        qDebug() << "Joystick Found: " << SDL_JoystickName(current);
    }

    connect(&testTimer, SIGNAL(timeout()), this, SLOT(processQueue()));
    controller.readSettings(settings);
}

void SDLEventQueue::setBusDevice(ScpBusDevice *busDevice)
{
    controller.setBusDevice(busDevice);
}

SDLEventQueue::~SDLEventQueue()
{
    if (sdlActive)
    {
        SDL_Quit();
        for (int i=SDL_NumJoysticks()-1; i >= 0; i--)
        {
            if (sdlJoysticks.contains(i))
            {
                SDL_Joystick *current = sdlJoysticks.value(i);
                if (current && SDL_JoystickGetAttached(current))
                {
                    SDL_JoystickClose(current);
                }
            }
        }
    }
}

void SDLEventQueue::initializeSDL()
{
    SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
    //SDL_JoystickEventState(SDL_ENABLE);
    /*SDL_Window *window;
    window = SDL_CreateWindow(
            "An SDL2 window",                  // window title
            SDL_WINDOWPOS_UNDEFINED,           // initial x position
            SDL_WINDOWPOS_UNDEFINED,           // initial y position
            640,                               // width, in pixels
            480,                               // height, in pixels
            SDL_WINDOW_OPENGL                  // flags - see below
        );
    */

    sdlActive = true;
}

void SDLEventQueue::processQueue()
{
    SDL_Event event;
    while (SDL_PollEvent(&event) > 0)
    {
        //qDebug() << "PROCESS QUEUE";

        switch (event.type)
        {
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
            {
                //qDebug() << "DELETE THIS";
                if (joystickMap.contains(event.jbutton.which))
                {
                    controller.buttonEvent(event.jbutton.button,
                                           event.type == SDL_JOYBUTTONDOWN ? true : false);
                }

                break;
            }
            case SDL_JOYAXISMOTION:
            {
                if (joystickMap.contains(event.jaxis.which))
                {
                    controller.axisEvent(event.jaxis.axis, event.jaxis.value);
                }

                break;
            }
            case SDL_JOYHATMOTION:
            {
                if (joystickMap.contains(event.jhat.which))
                {
                    controller.hatEvent(event.jhat.value);
                }

                break;
            }
            case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym == SDLK_RETURN)
                {
                    quitSDL();
                    break;
                }
            }
            case SDL_WINDOWEVENT_CLOSE:
            {
                quitSDL();
                break;
            }
            case SDL_QUIT:
            {
                quitSDL();
                break;
            }
        }
    }

    controller.outputReport();

    if (sdlActive)
    {
        if (!testTimer.isActive())
        {
            testTimer.start();
        }
    }
}

void SDLEventQueue::quitSDL()
{
    if (sdlActive)
    {
        SDL_Quit();
        sdlActive = false;
        testTimer.stop();
        emit closed();
    }
}

void SDLEventQueue::startProcessing()
{
    testTimer.start();
}

void SDLEventQueue::stopProcessing()
{
    testTimer.stop();
}

InputController* SDLEventQueue::getController()
{
    return &this->controller;
}

void SDLEventQueue::changePollRate(int pollRate)
{
    if (pollRate >= 1 && pollRate <= 10)
    {
        testTimer.setInterval(10);
        if (testTimer.isActive())
        {
            testTimer.start();
        }
    }
}

