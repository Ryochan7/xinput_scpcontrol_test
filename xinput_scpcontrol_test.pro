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
    testwindow.cpp \
    testmainwindownow.cpp

HEADERS += \
    scpdevice.h \
    sdleventqueue.h \
    inputcontroller.h \
    testwindow.h \
    testmainwindownow.h

INCLUDEPATH += "C:\Users\Travis\Downloads\antimicro\SDL2-2.0.4\x86_64-w64-mingw32\include"

LIBS += -lwinusb -lsetupapi -ladvapi32 -lhid -L"C:/Users/Travis/Downloads/antimicro/SDL2-2.0.4/i686-w64-mingw32/lib" -lSDL2

FORMS += \
    testmainwindownow.ui
