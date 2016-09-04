#include <QtGlobal>

#ifdef Q_OS_WIN
  #include <SDL2/SDL.h>
  #undef main
#endif


//#include <QCoreApplication>
#include <QApplication>

#include <QString>
#include <QTimer>
#include <QThread>
#include <QDebug>
#include <windows.h>

#include "scpdevice.h"
#include "sdleventqueue.h"
#include "testmainwindownow.h"

const QString SCP_BUS_CLASS_GUID = "{F679F562-3164-42CE-A4DB-E7DDBE723909}";

int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);
    QApplication a(argc, argv);
    ScpBusDevice devtest(SCP_BUS_CLASS_GUID);
    devtest.open();
    devtest.start();
    devtest.plugin(1);
    //devtest.plugin(2);

    //QTimer::singleShot(6000, &a, SLOT(quit()));
    SDLEventQueue sdlEventHandler(&devtest);
    QThread workthread;
    sdlEventHandler.moveToThread(&workthread);
    QTimer::singleShot(100, &sdlEventHandler, SLOT(processQueue()));
    QObject::connect(&sdlEventHandler, SIGNAL(closed()), &a, SLOT(quit()));
    TestMainWindowNOw winTest;
    QObject::connect(&a, SIGNAL(aboutToQuit()), &workthread, SLOT(quit()));
    QObject::connect(&workthread, SIGNAL(finished()), &sdlEventHandler, SLOT(stopProcessing()));
    workthread.start();
    winTest.show();
    //sdlEventHandler.processQueue();
    int result = a.exec();
    workthread.wait();
    //devtest.unplug(2);
    devtest.unplug(1);
    return result;
}

