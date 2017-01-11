#include <QDebug>
#include <QLibrary>
#include <windows.h>
#include <setupapi.h>
extern "C" {
#include <hidsdi.h>
}

#include "scpdevice.h"

ScpBusDevice::ScpBusDevice()
{
    m_IsActive = false;
    m_path = "";
    m_fileHandle = 0;
    m_offset = 0;
}

ScpBusDevice::ScpBusDevice(QString deviceClass) :
    m_deviceClass(deviceClass)
{
    m_IsActive = false;
    m_path = "";
    m_fileHandle = 0;
    m_offset = 0;
}

ScpBusDevice::~ScpBusDevice()
{
    unplugAll();
    close();
}

bool ScpBusDevice::isActive()
{
    return m_IsActive;
}

QString ScpBusDevice::path()
{
    return m_path;
}

bool ScpBusDevice::open(int instance)
{
    QString tempDevicePath;
    if (find(m_deviceClass, tempDevicePath, instance))
    {
        m_offset += instance * busWidth;
        open(tempDevicePath);
    }

    return m_IsActive;
}

bool ScpBusDevice::open(QString devicePath)
{
    m_path = devicePath.toUpper();

    if (getDeviceHandle(m_path))
    {
        m_IsActive = true;
    }

    return m_IsActive;
}

bool ScpBusDevice::start()
{
    return m_IsActive;
}

bool ScpBusDevice::stop()
{
    m_IsActive = false;

    if (m_fileHandle)
    {
        CloseHandle(m_fileHandle);
        m_fileHandle = 0;
    }

    return true;
}


bool ScpBusDevice::close()
{
    return stop();
}

bool ScpBusDevice::plugin(unsigned int controllerNumber)
{
    bool result = false;
    if (controllerNumber == 0 || controllerNumber > 4)
    {
        result = false;
    }

    unsigned int Serial = controllerNumber + m_offset;

    DWORD transferred = 0;

    byte buffer[16];
    memset(&buffer, 0x00, sizeof(buffer));
    buffer[0] = 0x10;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    buffer[4] = (byte)((Serial) & 0xFF);
    buffer[5] = (byte)((Serial >> 8) & 0xFF);
    buffer[6] = (byte)((Serial >> 16) & 0xFF);
    buffer[7] = (byte)((Serial >> 24) & 0xFF);

    if (DeviceIoControl(m_fileHandle, 0x2A4000, buffer, 16, 0, 0, &transferred, 0))
    {
        result = true;
    }

    return result;
}

bool ScpBusDevice::unplug(unsigned int controllerNumber)
{
    bool result = false;
    if (controllerNumber == 0 || controllerNumber > 4)
    {
        result = false;
    }

    unsigned int Serial = controllerNumber + m_offset;

    DWORD transferred = 0;

    byte buffer[16];
    memset(&buffer, 0x00, sizeof(buffer));
    buffer[0] = 0x10;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    buffer[4] = (byte)((Serial) & 0xFF);
    buffer[5] = (byte)((Serial >> 8) & 0xFF);
    buffer[6] = (byte)((Serial >> 16) & 0xFF);
    buffer[7] = (byte)((Serial >> 24) & 0xFF);

    if (DeviceIoControl(m_fileHandle, 0x2A4004, buffer, 16, 0, 0,
                        &transferred, 0))
    {
        result = true;
    }

    return result;
}

bool ScpBusDevice::unplugAll()
{
    bool result = false;

    DWORD transferred = 0;

    byte buffer[16];
    memset(&buffer, 0x00, sizeof(buffer));
    buffer[0] = 0x10;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    if (DeviceIoControl(m_fileHandle, 0x2A4004, buffer, 16, 0, 0,
                        &transferred, 0))
    {
        result = true;
    }

    return result;
}

void ScpBusDevice::generateReport(byte *&output)
{
    output = new byte[20];
    memset(output, 0x00, sizeof(*output) * 20);

    output[0] = 0x00;                                 // Message type (input report)
    output[1] = 0x14;                                 // Message size (20 bytes)

    unsigned int buttonsTest = 0;
    buttonsTest = buttonsTest | (1 << 0);
    buttonsTest = buttonsTest | (1 << 3);
    buttonsTest = buttonsTest | (1 << 4);
    buttonsTest = buttonsTest | (1 << 5);
    buttonsTest = buttonsTest | (1 << 14);

    output[2] = (byte)(buttonsTest & 0xFF);       // Buttons low
    output[3] = (byte)(buttonsTest >> 8 & 0xFF);  // Buttons high

    output[4] = 0;                          // Left trigger
    output[5] = 200;                         // Right trigger

    output[6] = (byte)(32000 & 0xFF);            // Left stick X-axis low
    output[7] = (byte)(32000 >> 8 & 0xFF);       // Left stick X-axis high
    output[8] = (byte)(32000 & 0xFF);            // Left stick Y-axis low
    output[9] = (byte)(32000 >> 8 & 0xFF);       // Left stick Y-axis high

    output[10] = (byte)(32000 & 0xFF);          // Right stick X-axis low
    output[11] = (byte)(32000 >> 8 & 0xFF);     // Right stick X-axis high
    output[12] = (byte)(32000 & 0xFF);          // Right stick Y-axis low
    output[13] = (byte)(32000 >> 8 & 0xFF);     // Right stick Y-axis high
}

bool ScpBusDevice::report(unsigned int controllerNumber, byte *input, byte *output)
{
    bool result = false;

    DWORD transferred = 0;
    byte fullreport[28];
    memset(&fullreport, 0x00, sizeof(fullreport));

    unsigned int Serial = controllerNumber + m_offset;

    fullreport[0] = 0x1C;
    fullreport[4] = (byte)((Serial) & 0xFF);
    fullreport[5] = (byte)((Serial >> 8) & 0xFF);
    fullreport[6] = (byte)((Serial >> 16) & 0xFF);
    fullreport[7] = (byte)((Serial >> 24) & 0xFF);

    memcpy(fullreport + 8, input, sizeof(*input) * 20);
    bool status = DeviceIoControl(m_fileHandle, 0x2A400C, fullreport, 28,
                                  output, 8, &transferred, 0);

    //int lastError = GetLastError();
    if (status && transferred > 0)
    {
        result = true;
    }

    return result;
}

bool ScpBusDevice::getDeviceHandle(QString path)
{
    //int lastError = 0;
    wchar_t tempPath[1024];
    memset(&tempPath, 0, sizeof(tempPath));
    path.toWCharArray(tempPath);
    //int lastError = GetLastError();
    m_fileHandle = CreateFileW(tempPath, (GENERIC_WRITE | GENERIC_READ), FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
    if (m_fileHandle == 0 || m_fileHandle == INVALID_HANDLE_VALUE)
    {
        m_fileHandle = 0;
    }

    //lastError = GetLastError();

    return !(m_fileHandle == 0);
}

bool ScpBusDevice::find(QUuid target, QString &path, int instance)
{
    bool result = false;

    PSP_INTERFACE_DEVICE_DETAIL_DATA detailDataBuffer = 0;
    HDEVINFO deviceInfoSet = 0;

    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    memset(&deviceInterfaceData, 0, sizeof(SP_DEVICE_INTERFACE_DATA));

    SP_DEVINFO_DATA da;
    memset(&da, 0, sizeof(da));

    GUID tempGUID = static_cast<GUID>(target);
    GUID falling;
    HidD_GetHidGuid(&falling);
    ULONG bufferSize = 0;
    int memberIndex = 0;
    deviceInfoSet = SetupDiGetClassDevs(&tempGUID, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    deviceInterfaceData.cbSize = da.cbSize = sizeof(deviceInterfaceData);
    while (SetupDiEnumDeviceInterfaces(deviceInfoSet, 0, &tempGUID, memberIndex, &deviceInterfaceData))
    {
        SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, 0, 0, &bufferSize, &da);
        {
            detailDataBuffer = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(bufferSize);
            detailDataBuffer->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

            if (SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, detailDataBuffer, bufferSize, &bufferSize, &da))
            {
                path = QString::fromWCharArray(detailDataBuffer->DevicePath).toUpper();
                free(detailDataBuffer);
                //qDebug() << "GUID: " << QUuid(da.ClassGuid).toString() << " | PATH: " << path;

                if (memberIndex == instance)
                {
                    //result = false;
                    result = true;
                    //return true;
                }

                //if (path == QString("\\\\?\\HID#VID_045E&PID_028E&IG_00#7&2E541179&0&0000#{4D1E55B2-F16F-11CF-88CB-001111000030}"))
                /*if (path == QString("\\\\?\\HID#VID_0810&PID_0001&COL02#6&2A28EB21&0&0001#{4D1E55B2-F16F-11CF-88CB-001111000030}")
                    || path == QString("\\\\?\\HID#VID_0810&PID_0001&COL01#6&2A28EB21&0&0000#{4D1E55B2-F16F-11CF-88CB-001111000030}"))
                {
                    qDebug() << "I'm Falling";
                    wchar_t tempPath[1024];
                    memset(&tempPath, 0, sizeof(tempPath));
                    QString(path).toWCharArray(tempPath);

                    HANDLE bacon = CreateFileW(tempPath, (GENERIC_WRITE | GENERIC_READ), FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
                    if (bacon == 0 || bacon == INVALID_HANDLE_VALUE)
                    {
                        int lastError = GetLastError();
                        qDebug() << "LKJFDSLKJDSLKJDSFLKJSDLKJF " << bacon << " " << lastError << "\n\n\n\n\n";
                    }
                }
                */
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
    //return false;
}

