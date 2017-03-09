#ifndef DS4DEVICETEST_H
#define DS4DEVICETEST_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QQueue>
#include <QSettings>
#include <windows.h>

#include "scpdevice.h"
#include "axiscurve.h"
#include "defaultsettings.h"

class DS4DeviceTest : public QObject
{
    Q_OBJECT
public:
    enum X360Buttons {
        NONE = 0,
        DPAD_UP = (1 << 0),
        DPAD_DOWN = (1 << 1),
        DPAD_LEFT = (1 << 2),
        DPAD_RIGHT = (1 << 3),
        START = (1 << 4),
        BACK = (1 << 5),
        LEFTSTICK = (1 << 6),
        RIGHTSTICK = (1 << 7),
        LEFTBUMPER = (1 << 8),
        RIGHTBUMPER = (1 << 9),
        GUIDE = (1 << 10),
        A = (1 << 12),
        B = (1 << 13),
        X = (1 << 14),
        Y = (1 << 15),
    };

    enum DS4Buttons {
        DS4_SQUARE_BUTTON = 0,
        DS4_CROSS_BUTTON = 1,
        DS4_CIRCLE_BUTTON = 2,
        DS4_TRIANGLE_BUTTON = 3,
        DS4_L1_BUTTON = 4,
        DS4_R1_BUTTON = 5,
        DS4_L2_BUTTON = 6,
        DS4_R2_BUTTON = 7,
        DS4_SHARE_BUTTON = 8,
        DS4_OPTIONS_BUTTON = 9,
        DS4_L3_BUTTON = 10,
        DS4_R3_BUTTON = 11,
        DS4_PS_BUTTON = 12,
        DS4_TOUCHPAD_BUTTON = 13,
    };

    enum DS4_Axes {
        DS4_LX = 0,
        DS4_LY,
        DS4_RX,
        DS4_RY,
        DS4_LTRIGGER,
        DS4_RTRIGGER,
    };

    enum DS4_Hat_Dir {
        DS4_HAT_CENTERED = 0x00,
        DS4_HAT_UP = 0x01,
        DS4_HAT_RIGHT = 0x02,
        DS4_HAT_DOWN = 0x04,
        DS4_HAT_LEFT = 0x08,
    };

    typedef struct _MaxZoneValues {
        int maxZoneNegValue;
        int maxZonePosValue;
    } MaxZoneValues;

    static const int XINPUT_AXIS_MAX; // = 32767;
    static const int XINPUT_AXIS_MIN; //= -32768;
    static const int INPUT_AXIS_MAX;
    static const int INPUT_AXIS_MIN;
    static const int INPUT_HALF_AXIS_MAX;
    static const int INPUT_HALF_AXIS_MIN;
    static const int MAXSMOOTHINGSIZE;

    explicit DS4DeviceTest(HANDLE fileHandle, ScpBusDevice *busDevice, QObject *parent = 0);

    void setBusDevice(ScpBusDevice *busDevice);
    void readSettings(QSettings *settings);

    void generateXinputReport(byte *&report);
    void outputReport();

protected:
    static const unsigned int MAXBUTTONS = 16;
    static const unsigned int MAXAXES = 6;
    static const unsigned int DEFAULTTRIGDEAD = ProgramDefaults::leftTriggerDeadZone;

    int computeSmoothedValue(int axis);
    int getCurvedAxisValue(AxisCurve::Type curve, int value, int axis);
    AxisCurve::Type getCurveFromString(QString value);
    int calculateAxisValueAfterDead(int axis, int value);
    void calculateStickValuesAfterDead(int axis1, int axis2, int axis1Value,
                                       int axis2Value, int &outAxis1Value, int &outAxis2Value);
    int calculateFlippedAxisValue(int value);

    inline void readInputReport();

    ScpBusDevice *busDevice;
    HANDLE m_fileHandle;
    int xinputIndex;
    bool buttons[14];
    int axes[MAXAXES];
    int hat;
    QTimer testTimer;
    QTime timeit;
    QQueue<int> smoothingBuffer[MAXAXES];
    bool smoothingEnabled[MAXAXES];
    double smoothingWeights[MAXAXES];
    int smoothingSizes[MAXAXES];
    AxisCurve::Type axisCurves[MAXAXES];
    double axisScales[MAXAXES];
    int axisDeadZones[MAXAXES];
    int axisAntiDeadZones[MAXAXES];
    int axisMaxZones[MAXAXES];
    double axisSens[MAXAXES];
    OVERLAPPED olu;
    OVERLAPPED olw;
    byte inputReport[64];
    byte outputDeviceReport[32];
    bool isWaitingOverLap;
    bool isWaitingWriteOverLap;
    byte rightLightFastRumbleMotor;
    byte leftHeavySlowRumbleMotor;

signals:

public slots:
    void changeSmoothingStatus(int axis, bool enabled);
    void changeSmoothingSize(int axis, int size);
    void changeSmoothingWeight(int axis, double weight);
    void changeAxisCurve(int axis, int curve);
    void changeAxisScale(int axis, double value);
    void changeAxisDeadZonePercentage(int axis, int value);
    void changeAxisAntiDeadZonePercentage(int axis, int value);
    void changeAxisMaxZonePercentage(int axis, int value);
    void changeAxisSens(int axis, double value);
    int getAxisMaxZoneNegValue(int axis);
    int getAxisMaxZonePosValue(int axis);
    void changePollRate(int pollRate);

    void readControllerState();
};

#endif // DS4DEVICETEST_H
