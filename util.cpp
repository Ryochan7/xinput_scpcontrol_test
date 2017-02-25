#include <QDebug>
#include <QLibrary>
#include <QUuid>
#include <QThread>
#include <windows.h>
#include <setupapi.h>
extern "C" {
#include <hidsdi.h>
}

#include "ds4devicetest.h"
#include "util.h"
#include "defaultsettings.h"

Util::Util(QObject *parent) : QObject(parent)
{
    //testTimer.setParent(this);
    //testTimer.setInterval(10);
    //testTimer.setInterval(settings->value("pollRate", ProgramDefaults::pollRate).toInt());
    //testTimer.setSingleShot(false);
    //testTimer.setTimerType(Qt::PreciseTimer);
}

QList<DS4DeviceTest*>* Util::getControllers()
{
    return &controllers;
}

DS4DeviceTest* Util::getController()
{
    DS4DeviceTest *result = 0;
    if (!controllers.isEmpty())
    {
        result = controllers.first();
    }

    return result;
}

bool Util::discoverDS4Controllers(ScpBusDevice *busDevice)
{
    bool result = false;

    PSP_INTERFACE_DEVICE_DETAIL_DATA detailDataBuffer = 0;
    HDEVINFO deviceInfoSet = 0;

    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    memset(&deviceInterfaceData, 0, sizeof(SP_DEVICE_INTERFACE_DATA));

    SP_DEVINFO_DATA da;
    memset(&da, 0, sizeof(da));

    GUID mainHidGuid;
    HidD_GetHidGuid(&mainHidGuid);
    ULONG bufferSize = 0;
    int memberIndex = 0;
    deviceInfoSet = SetupDiGetClassDevs(&mainHidGuid, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    deviceInterfaceData.cbSize = da.cbSize = sizeof(deviceInterfaceData);

    while (SetupDiEnumDeviceInterfaces(deviceInfoSet, 0, &mainHidGuid, memberIndex, &deviceInterfaceData))
    {
        SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, 0, 0, &bufferSize, &da);
        {
            detailDataBuffer = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(bufferSize);
            detailDataBuffer->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

            if (SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, detailDataBuffer, bufferSize, &bufferSize, &da))
            {
                QString path = QString::fromWCharArray(detailDataBuffer->DevicePath).toUpper();
                //qDebug() << "GUID: " << QUuid(da.ClassGuid).toString() << " | PATH: " << path;
                free(detailDataBuffer);

                wchar_t tempPath[1024];
                memset(&tempPath, 0, sizeof(tempPath));
                QString(path).toWCharArray(tempPath);
                HANDLE fileHandle = CreateFileW(tempPath, (GENERIC_WRITE | GENERIC_READ), FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
                if (fileHandle == 0 || fileHandle == INVALID_HANDLE_VALUE)
                {
                    result = false;
                }
                else
                {
                    int myVendorID = 0;
                    int myProductID = 0;
                    HIDD_ATTRIBUTES devAttributes;
                    HidD_GetAttributes(fileHandle, &devAttributes);
                    myVendorID = devAttributes.VendorID;
                    myProductID = devAttributes.ProductID;
                    //qDebug() << "JLKFDLJK: " << myVendorID << " | " << myProductID;

                    CloseHandle(fileHandle);

                    if (myVendorID == 0x54c && myProductID == 0x5c4)
                    {
                        //HANDLE fileHandle = CreateFileW(tempPath, (GENERIC_WRITE | GENERIC_READ), FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
                        //HANDLE fileHandle = CreateFileW(tempPath, (GENERIC_WRITE | GENERIC_READ), FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
                        fileHandle = CreateFileW(tempPath, (GENERIC_WRITE | GENERIC_READ), 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
                        if (fileHandle == 0 || fileHandle == INVALID_HANDLE_VALUE)
                        {
                            int lastError = GetLastError();
                            qDebug() << "LKJFDSLKJDSLKJDSFLKJSDLKJF " << fileHandle << " " << lastError << "\n\n\n\n\n";
                        }
                        else
                        {
                            HidD_SetNumInputBuffers(fileHandle, 2);
                            DS4DeviceTest *controller = new DS4DeviceTest(fileHandle, busDevice);
                            QThread *readerThread = new QThread(this);
                            controller->moveToThread(readerThread);
                            readerThread->start(QThread::HighPriority);
                            result = true;
                            controllers.append(controller);
                        }
                    }
                }
            }
            else
            {
                free(detailDataBuffer);
            }
        }

        memberIndex++;
    }

    if (deviceInfoSet)
    {
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
    }

    return result;
}

void Util::stopControllers()
{
    QListIterator<DS4DeviceTest*> iter(controllers);
    while (iter.hasNext())
    {
        DS4DeviceTest *temp = iter.next();
        if (temp)
        {
            temp->thread()->quit();
            temp->thread()->wait();
            temp->deleteLater();
            temp = 0;
        }
    }

    controllers.clear();
}

void Util::changePollRate(int pollRate)
{
    if (pollRate >= 1 && pollRate <= 16)
    {
        /*testTimer.setInterval(pollRate);
        if (testTimer.isActive())
        {
            testTimer.start();
        }
        */

        QListIterator<DS4DeviceTest*> iter(controllers);
        while (iter.hasNext())
        {
            DS4DeviceTest *temp = iter.next();
            if (temp)
            {
                QMetaObject::invokeMethod(temp, "changePollRate", Qt::AutoConnection, Q_ARG(int, pollRate));
            }
        }
    }
}
