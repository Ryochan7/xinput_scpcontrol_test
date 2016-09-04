#ifndef SCPDEVICE_H
#define SCPDEVICE_H

#include <QString>
#include <QUuid>
#include <windef.h>
#include <QByteArray>
#include <windows.h>

class ScpBusDevice
{
public:
    ScpBusDevice();
    ScpBusDevice(QString deviceClass);

    ~ScpBusDevice();

    bool isActive();
    QString path();
    virtual bool open(int instance = 0);
    virtual bool open(QString devicePath);
    virtual bool start();
    virtual bool stop();
    virtual bool close();
    virtual bool plugin(unsigned int controllerNumber);
    virtual bool unplug(unsigned int controllerNumber);
    virtual bool unplugAll();
    virtual bool report(unsigned int controllerNumber, byte *input, byte *output);
    void generateReport(byte *&output);

protected:
    bool getDeviceHandle(QString path);
    bool find(QUuid target, QString &path, int instance = 0);

    bool m_IsActive;
    QString m_path;
    QUuid m_deviceClass;
    HANDLE m_fileHandle;
    int m_offset;
    static const int busWidth = 4;
};

#endif // SCPDEVICE_H
