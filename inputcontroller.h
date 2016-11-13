#ifndef INPUTCONTROLLER_H
#define INPUTCONTROLLER_H

#include <QObject>
#include <QString>
#include <QQueue>
#include <QSettings>

#include "scpdevice.h"
#include "axiscurve.h"

class InputController : public QObject
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

    enum PS1Buttons {
        TRIANGLE = 0,
        CIRCLE = 1,
        CROSS = 2,
        SQUARE = 3,
        L2 = 4,
        R2 = 5,
        L1 = 6,
        R1 = 7,
        SELECT = 8,
        STARTPS1 = 9,
        L3 = 10,
        R3 = 11,
    };

    /*enum AxisCurves {
        Linear,
        EnhancedPrecision,
        Quadratic,
        Cubic,
        Disabled,
    };
    */

    explicit InputController(ScpBusDevice *busDevice, QObject *parent = 0);

    void buttonEvent(int buttonIndex, bool value);
    void axisEvent(int axisIndex, int value);
    void hatEvent(int value);
    void generateXinputReport(byte *&report);
    void outputReport();
    void setBusDevice(ScpBusDevice *busDevice);
    void readSettings(QSettings *settings);

    static const int AXIS_MAX; // = 32767;
    static const int AXIS_MIN; //= -32768;
    static const int MAXSMOOTHINGSIZE;


public slots:
    void changeSmoothingStatus(int axis, bool enabled);
    void changeSmoothingSize(int axis, int size);
    void changeSmoothingWeight(int axis, double weight);
    void changeAxisCurve(int axis, int curve);
    void changeAxisScale(int axis, double value);
    void changeAxisDeadZonePercentage(int axis, int value);
    void changeAxisAntiDeadZonePercentage(int axis, int value);

protected:
    int computeSmoothedValue(int axis);
    int getCurvedAxisValue(AxisCurve::Type curve, int value, int axis);
    AxisCurve::Type getCurveFromString(QString value);
    int calculateAxisValueAfterDead(int axis, int value);
    void calculateStickValuesAfterDead(int axis1, int axis2, int axis1Value,
                                       int axis2Value, int &outAxis1Value, int &outAxis2Value);

    static const unsigned int MAXBUTTONS = 16;
    static const unsigned int MAXAXES = 4;

    ScpBusDevice *busDevice;
    int xinputIndex;
    bool buttons[MAXBUTTONS];
    int axes[MAXAXES];
    int hat;
    QQueue<int> smoothingBuffer[MAXAXES];
    bool smoothingEnabled[MAXAXES];
    double smoothingWeights[MAXAXES];
    int smoothingSizes[MAXAXES];
    AxisCurve::Type axisCurves[MAXAXES];
    double axisScales[MAXAXES];
    int axisDeadZones[MAXAXES];
    int axisAntiDeadZones[MAXAXES];

signals:

public slots:

};

#endif // INPUTCONTROLLER_H
