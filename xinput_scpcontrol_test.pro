#QT += core
#QT -= gui

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = xinput_scpcontrol_test
#CONFIG += console
#CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    scpdevice.cpp \
    sdleventqueue.cpp \
    inputcontroller.cpp \
    testmainwindownow.cpp \
    axiscurve.cpp

HEADERS += \
    scpdevice.h \
    sdleventqueue.h \
    inputcontroller.h \
    testmainwindownow.h \
    defaultsettings.h \
    axiscurve.h

exists($$PWD/SDL2-2.0.5/i686-w64-mingw32/include) {
    INCLUDEPATH += "$$PWD/SDL2-2.0.5/i686-w64-mingw32/include"
}

LIBS += -lwinusb -lsetupapi -ladvapi32 -lhid -lSDL2

exists($$PWD/SDL2-2.0.5/i686-w64-mingw32/lib) {
    LIBS += -L"$$PWD/SDL2-2.0.5/i686-w64-mingw32/lib"
}

FORMS += \
    testmainwindownow.ui
