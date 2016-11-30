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

        if (SDL_IsGameController(i))
        {
            SDL_GameController *controller = SDL_GameControllerOpen(i);
            SDL_Joystick *current = SDL_GameControllerGetJoystick(controller);
            //SDL_Joystick *current = SDL_JoystickOpen(i);
            sdlJoysticks.insert(i, current);
            SDL_JoystickID tempID = SDL_JoystickInstanceID(current);

            QString tempname = QString::fromLocal8Bit(SDL_JoystickName(current));

            SDL_JoystickGUID tempGUIDa = SDL_JoystickGetGUID(current);
            char guidString[65] = {'0'};
            SDL_JoystickGetGUIDString(tempGUIDa, guidString, sizeof(guidString));
            QString temp = QString(guidString);
            qDebug() << "Joystick Found: " << tempname << "" <<
                        temp;

            //if (tempname == QStringLiteral("Twin USB Joystick"))
            //if (tempname == QStringLiteral("MP-8866 Dual USB Joypad"))
            //{
                /*SDL_JoystickID tempID = SDL_JoystickInstanceID(current);
                if (!joystickMap.contains(tempID))
                {
                    joystickMap.insert(tempID, current);
                }
            }
            */

            if (!joystickMap.contains(tempID) && !tempname.contains("XInput Controller"))
            {
                joystickMap.insert(tempID, current);
            }
        }
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

    SDL_GameControllerAddMapping("4c05c405000000000000504944564944,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:a4,rightx:a2,righty:a5,start:b9,x:b0,y:b3,platform:Windows,");
    SDL_GameControllerAddMapping("10080100000000000000504944564944,Twin USB Joystick,a:b2,b:b1,x:b3,y:b0,back:b8,start:b9,leftstick:b10,rightstick:b11,leftshoulder:b6,rightshoulder:b7,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a3,righty:a2,lefttrigger:b4,righttrigger:b5,platform:Windows,");
    SDL_GameControllerAddMapping("25096688000000000000504944564944,MP-8866 Dual USB Joypad,a:b2,b:b1,x:b3,y:b0,back:b9,start:b8,leftstick:b10,rightstick:b11,leftshoulder:b6,rightshoulder:b7,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b4,righttrigger:b5,platform:Windows,");
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
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            {
                //qDebug() << "DELETE THIS";
                if (joystickMap.contains(event.cbutton.which))
                {
                    controller.buttonEvent(event.cbutton.button,
                                           event.type == SDL_CONTROLLERBUTTONDOWN ? true : false);
                }

                break;
            }
            case SDL_CONTROLLERAXISMOTION:
            {
                if (joystickMap.contains(event.caxis.which))
                {
                    if (event.caxis.axis == 4)
                    {
                        //qDebug() << "WELL I WELL " << event.caxis.value;
                    }

                    controller.axisEvent(event.caxis.axis, event.caxis.value);
                }

                break;
            }

            /*case SDL_JOYHATMOTION:
            {
                if (joystickMap.contains(event.jhat.which))
                {
                    controller.hatEvent(event.jhat.value);
                }

                break;
            }*/

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

