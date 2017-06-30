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
#include <QSettings>

#include "scpdevice.h"
#include "scpbuscontainer.h"
#include "sdleventqueue.h"
#include "testmainwindownow.h"

const QString SCP_BUS_CLASS_GUID = "{F679F562-3164-42CE-A4DB-E7DDBE723909}";

int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);
    QApplication a(argc, argv);
    QThread busThread;
    ScpBusContainer devcontainer;
    devcontainer.moveToThread(&busThread);
    devcontainer.initBusDevice();
    devcontainer.getBusDevice()->plugin(1);
    ScpBusDevice *devtest = devcontainer.getBusDevice();
    /*ScpBusDevice devtest(SCP_BUS_CLASS_GUID);
    devtest.open();
    devtest.start();
    devtest.plugin(1);
    */
    //devtest.plugin(2);

    QSettings programSettings(qApp->applicationDirPath().append("/").append("settings.ini"),
                              QSettings::IniFormat);

    //Util util;
    //util.discoverDS4Controllers(&devtest, &programSettings);
    //util.changePollRate(programSettings.value("pollRate", ProgramDefaults::pollRate).toInt());

    SDLEventQueue sdlEventHandler(devtest, &programSettings);
    QThread workerthread;
    sdlEventHandler.moveToThread(&workerthread);

    QTimer::singleShot(100, &sdlEventHandler, SLOT(processQueue()));
    QObject::connect(&sdlEventHandler, SIGNAL(closed()), &a, SLOT(quit()));
    TestMainWindowNOw winTest(&sdlEventHandler, &programSettings);
    QObject::connect(&a, SIGNAL(aboutToQuit()), &workerthread, SLOT(quit()));
    QObject::connect(&workerthread, SIGNAL(finished()), &sdlEventHandler, SLOT(stopProcessing()));

    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    workerthread.start(QThread::HighPriority);
    winTest.show();
    int result = a.exec();
    workerthread.wait();
    devcontainer.getBusDevice()->unplugAll();
    //devtest.unplug(1);
    return result;
}

