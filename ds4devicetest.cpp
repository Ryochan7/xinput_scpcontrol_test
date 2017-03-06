#include <QDebug>
#include <QtMath>

#include "ds4devicetest.h"

const int DS4DeviceTest::INPUT_AXIS_MIN = 0;
const int DS4DeviceTest::INPUT_AXIS_MAX = 255;
const int DS4DeviceTest::INPUT_HALF_AXIS_MIN = -128;
const int DS4DeviceTest::INPUT_HALF_AXIS_MAX = 127;
const int DS4DeviceTest::XINPUT_AXIS_MIN = -32768;
const int DS4DeviceTest::XINPUT_AXIS_MAX = 32767;
const int DS4DeviceTest::MAXSMOOTHINGSIZE = 30;


DS4DeviceTest::DS4DeviceTest(HANDLE fileHandle, ScpBusDevice *outDevice, QObject *parent) : QObject(parent),
    m_fileHandle(fileHandle)
{
    this->busDevice = outDevice;
    xinputIndex = 1;

    isWaitingOverLap = false;

    memset(&inputReport, 0, sizeof(inputReport));
    memset(&olu, 0, sizeof(olu));

    memset(&buttons, 0, sizeof(buttons));
    memset(&axes, 0, sizeof(axes));
    memset(&smoothingEnabled, 0, sizeof(smoothingEnabled));
    memset(&smoothingWeights, 0, sizeof(smoothingWeights));
    memset(&smoothingSizes, 0, sizeof(smoothingSizes));
    memset(&axisScales, 0, sizeof(axisScales));
    memset(&axisDeadZones, 0, sizeof(axisDeadZones));
    memset(&axisAntiDeadZones, 0, sizeof(axisAntiDeadZones));
    memset(&axisMaxZones, 0, sizeof(axisMaxZones));
    memset(&axisSens, 0, sizeof(axisSens));

    // Set default dead zone for triggers
    axisDeadZones[DS4_LTRIGGER] = DEFAULTTRIGDEAD;
    axisDeadZones[DS4_RTRIGGER] = DEFAULTTRIGDEAD;

    for (unsigned int i=0; i < MAXAXES; i++)
    {
        smoothingWeights[i] = 1.0;
    }

    for (unsigned int i=0; i < MAXAXES; i++)
    {
        smoothingSizes[i] = 1;
    }

    for (unsigned int i=0; i < MAXAXES; i++)
    {
        smoothingBuffer[i].prepend(0);
    }

    for (unsigned int i=0; i < MAXAXES; i++)
    {
        axisScales[i] = 1.0;
    }

    for (unsigned int i=0; i < MAXAXES; i++)
    {
        axisMaxZones[i] = 100;
    }

    hat = 0;

    testTimer.setParent(this);
    connect(&testTimer, SIGNAL(timeout()), this, SLOT(readControllerState()));
    testTimer.setInterval(10);
    testTimer.setSingleShot(false);
    testTimer.setTimerType(Qt::PreciseTimer);
    testTimer.start();
    timeit.restart();
}

void DS4DeviceTest::setBusDevice(ScpBusDevice *busDevice)
{
    this->busDevice = busDevice;
}

void DS4DeviceTest::readSettings(QSettings *settings)
{
    bool leftStickSmooth = settings->value("leftStickSmoothEnabled",
                                           ProgramDefaults::leftStickSmoothEnabled).toBool();
    if (leftStickSmooth)
    {
        changeSmoothingStatus(DS4_LX, true);
        changeSmoothingStatus(DS4_LY, true);
    }

    bool rightStickSmooth = settings->value("rightStickSmoothEnabled",
                                            ProgramDefaults::rightStickSmoothEnabled).toBool();
    if (rightStickSmooth)
    {
        changeSmoothingStatus(DS4_RX, true);
        changeSmoothingStatus(DS4_RY, true);
    }

    int leftStickSmoothSize = settings->value("leftStickSmoothSize",
                                              ProgramDefaults::leftStickSmoothSize).toInt();
    changeSmoothingSize(DS4_LX, leftStickSmoothSize);
    changeSmoothingSize(DS4_LY, leftStickSmoothSize);

    int rightStickSmoothSize = settings->value("rightStickSmoothSize",
                                               ProgramDefaults::rightStickSmoothSize).toInt();
    changeSmoothingSize(DS4_RX, rightStickSmoothSize);
    changeSmoothingSize(DS4_RY, rightStickSmoothSize);

    int leftStickSmoothWeight = settings->value("leftStickSmoothWeight",
                                                ProgramDefaults::leftStickSmoothWeight).toInt();
    changeSmoothingWeight(DS4_LX, leftStickSmoothWeight / 100.0);
    changeSmoothingWeight(DS4_LY, leftStickSmoothWeight / 100.0);

    double rightStickSmoothWeight = settings->value("rightStickSmoothWeight",
                                                    ProgramDefaults::rightStickSmoothWeight).toInt();
    changeSmoothingWeight(DS4_RX, rightStickSmoothWeight / 100.0);
    changeSmoothingWeight(DS4_RY, rightStickSmoothWeight / 100.0);

    QString leftStickCurve = settings->value("leftStickDefaultCurve",
                                             ProgramDefaults::leftStickDefaultCurve).toString();
    AxisCurve::Type leftCurveValue = getCurveFromString(leftStickCurve);
    axisCurves[DS4_LX] = leftCurveValue;
    axisCurves[DS4_LY] = leftCurveValue;

    QString rightStickCurve = settings->value("rightStickDefaultCurve",
                                              ProgramDefaults::rightStickDefaultCurve).toString();
    AxisCurve::Type rightCurveValue = getCurveFromString(rightStickCurve);
    axisCurves[DS4_RX] = rightCurveValue;
    axisCurves[DS4_RY] = rightCurveValue;

    int leftStickScale = settings->value("leftStickScale",
                                         ProgramDefaults::leftStickScale).toInt();

    changeAxisScale(DS4_LX, leftStickScale / 100.0);
    changeAxisScale(DS4_LY, leftStickScale / 100.0);

    int rightStickScale = settings->value("rightStickScale",
                                          ProgramDefaults::rightStickScale).toInt();

    changeAxisScale(DS4_RX, rightStickScale / 100.0);
    changeAxisScale(DS4_RY, rightStickScale / 100.0);

    int leftStickDeadZone = settings->value("leftStickDeadZone",
                                            ProgramDefaults::leftStickDeadZone).toInt();
    changeAxisDeadZonePercentage(DS4_LX, leftStickDeadZone);
    changeAxisDeadZonePercentage(DS4_LY, leftStickDeadZone);

    int rightStickDeadZone = settings->value("rightStickDeadZone",
                                             ProgramDefaults::rightStickDeadZone).toInt();
    changeAxisDeadZonePercentage(DS4_RX, rightStickDeadZone);
    changeAxisDeadZonePercentage(DS4_RY, rightStickDeadZone);

    int leftStickAntiDeadZone = settings->value("leftStickAntiDeadZone",
                                                ProgramDefaults::leftStickAntiDeadZone).toInt();
    changeAxisAntiDeadZonePercentage(DS4_LX, leftStickAntiDeadZone);
    changeAxisAntiDeadZonePercentage(DS4_LY, leftStickAntiDeadZone);

    int rightStickAntiDeadZone = settings->value("rightStickAntiDeadZone",
                                                 ProgramDefaults::rightStickAntiDeadZone).toInt();
    changeAxisAntiDeadZonePercentage(DS4_RX, rightStickAntiDeadZone);
    changeAxisAntiDeadZonePercentage(DS4_RY, rightStickAntiDeadZone);

    int leftStickMaxZone = settings->value("leftStickMaxZone",
                                           ProgramDefaults::leftStickMaxZone).toInt();
    changeAxisMaxZonePercentage(DS4_LX, leftStickMaxZone);
    changeAxisMaxZonePercentage(DS4_LY, leftStickMaxZone);

    int rightStickMaxZone = settings->value("rightStickMaxZone",
                                            ProgramDefaults::rightStickMaxZone).toInt();
    changeAxisMaxZonePercentage(DS4_RX, rightStickMaxZone);
    changeAxisMaxZonePercentage(DS4_RY, rightStickMaxZone);

    double leftStickSens = settings->value("leftStickSens",
                                           ProgramDefaults::leftStickSens).toDouble();
    changeAxisSens(DS4_LX, leftStickSens);
    changeAxisSens(DS4_LY, leftStickSens);

    double rightStickSens = settings->value("rightStickSens",
                                           ProgramDefaults::rightStickSens).toDouble();
    changeAxisSens(DS4_RX, rightStickSens);
    changeAxisSens(DS4_RY, rightStickSens);

    int leftTriggerDeadZone = settings->value("leftTriggerDeadZone",
                                            ProgramDefaults::leftTriggerDeadZone).toInt();
    changeAxisDeadZonePercentage(DS4_LTRIGGER, leftTriggerDeadZone);

    int rightTriggerDeadZone = settings->value("rightTriggerDeadZone",
                                            ProgramDefaults::rightTriggerDeadZone).toInt();
    changeAxisDeadZonePercentage(DS4_RTRIGGER, rightTriggerDeadZone);
}

void DS4DeviceTest::changeSmoothingStatus(int axis, bool enabled)
{
    if (axis >= 0 && axis < MAXAXES)
    {
        smoothingBuffer[axis].clear();
        smoothingEnabled[axis] = enabled;

        if (enabled)
        {
            int size = smoothingSizes[axis];
            for (int i=0; i < size; i++)
            {
                smoothingBuffer[axis].prepend(0);
            }
        }
    }
}

void DS4DeviceTest::changeSmoothingSize(int axis, int size)
{
    if (axis >= 0 && axis < MAXAXES &&
        size >= 0 && size <= MAXSMOOTHINGSIZE)
    {
        smoothingSizes[axis] = size;
        smoothingBuffer[axis].clear();
        for (int i=0; i < size; i++)
        {
            smoothingBuffer[axis].prepend(0);
        }
    }
}

void DS4DeviceTest::changeSmoothingWeight(int axis, double weight)
{
    if (axis >= 0 && axis < MAXAXES &&
        weight >= 0.0 && weight <= 1.0)
    {
        smoothingWeights[axis] = weight;
    }
}

void DS4DeviceTest::changeAxisCurve(int axis, int curve)
{
    if (axis >= 0 && axis < MAXAXES)
    {
        if (curve == 0)
        {
            axisCurves[axis] = AxisCurve::Linear;
        }
        else if (curve == 1)
        {
            axisCurves[axis] = AxisCurve::EnhancedPrecision;
        }
        else if (curve == 2)
        {
            axisCurves[axis] = AxisCurve::Quadratic;
        }
        else if (curve == 3)
        {
            axisCurves[axis] = AxisCurve::Cubic;
        }
        else if (curve == 4)
        {
            axisCurves[axis] = AxisCurve::Power;
        }
        else if (curve == 5)
        {
            axisCurves[axis] = AxisCurve::Disabled;
        }
    }
}

void DS4DeviceTest::changeAxisScale(int axis, double value)
{
    if (axis >= 0 && axis < MAXAXES &&
        value >= 0.0 && value <= 2.0)
    {
        if (value != axisScales[axis])
        {
            axisScales[axis] = value;
        }
    }
}

void DS4DeviceTest::changeAxisDeadZonePercentage(int axis, int value)
{
    if (axis >= 0 && axis < MAXAXES &&
        value >= 0 && value <= 100)
    {
        axisDeadZones[axis] = value;
    }
}

void DS4DeviceTest::changeAxisAntiDeadZonePercentage(int axis, int value)
{
    if (axis >= 0 && axis < MAXAXES &&
        value >= 0 && value <= 100)
    {
        axisAntiDeadZones[axis] = value;
    }
}

void DS4DeviceTest::changeAxisMaxZonePercentage(int axis, int value)
{
    if (axis >= 0 && axis < MAXAXES &&
        value >= 0 && value <= 100)
    {
        axisMaxZones[axis] = value;
    }
}

void DS4DeviceTest::changeAxisSens(int axis, double value)
{
    if (axis >= 0 && axis < MAXAXES &&
        value >= 0 && value <= 100.0)
    {
        axisSens[axis] = value;
    }
}

int DS4DeviceTest::getAxisMaxZoneNegValue(int axis)
{
    int result;

    if (axis >= 0 && axis < MAXAXES)
    {
        int axisMaxTemp = axisMaxZones[axis];

        int maxzoneNegDirValue = (axisMaxTemp / 100.0) * XINPUT_AXIS_MIN;
        result = maxzoneNegDirValue;
    }

    return result;
}

int DS4DeviceTest::getAxisMaxZonePosValue(int axis)
{
    int result;

    if (axis >= 0 && axis < MAXAXES)
    {
        int axisMaxTemp = axisMaxZones[axis];

        int maxzonePosDirValue = (axisMaxTemp / 100.0) * XINPUT_AXIS_MAX;
        result = maxzonePosDirValue;
    }

    return result;
}

int DS4DeviceTest::computeSmoothedValue(int axis)
{
    int result = axes[axis];

    if (smoothingEnabled[axis])
    {
        int smoothingSize = smoothingSizes[axis];
        if (smoothingBuffer[axis].size() >= smoothingSize)
        {
            smoothingBuffer[axis].removeLast();
        }

        smoothingBuffer[axis].prepend(axes[axis]);
        QListIterator<int> iter(smoothingBuffer[axis]);
        double currentWeight = 1.0;
        //double weightModifier = 1.0;
        double weightModifier = smoothingWeights[axis];
        double finalWeight = 0.0;
        double adjustedValue = 0.0;

        while (iter.hasNext())
        {
            int temp = iter.next();
            adjustedValue += temp * currentWeight;
            finalWeight += currentWeight;
            currentWeight *= weightModifier;
        }

        result = qFloor(adjustedValue / finalWeight);
    }

    return result;
}

int DS4DeviceTest::getCurvedAxisValue(AxisCurve::Type curve, int value, int axis)
{
    Q_UNUSED(axis);

    AxisCurve::Type tempCurve = curve;
    int result = value;

    switch (tempCurve)
    {
        case AxisCurve::Linear:
        {
            result = result;
            break;
        }
        case AxisCurve::EnhancedPrecision:
        {
            int sign = result > 0 ? 1 :-1;
            // Normalize axis value with asymmetric max points
            double temp = qAbs(result / ((sign > 0) ? 32767.0 : -32768.0));
            double outputTemp = temp;
            if (temp <= 0.4)
            {
                outputTemp = temp * 0.38;
            }
            else if (temp <= 0.75)
            {
                outputTemp = temp - 0.248;
            }
            else if (temp > 0.75)
            {
                outputTemp = (temp * 1.992) - 0.992;
            }

            // Take original sign into account and compute output value
            result = outputTemp * (sign > 0 ? 32767 : -32768);
            break;
        }
        case AxisCurve::Quadratic:
        {
            int sign = result > 0 ? 1 :-1;
            int temp = result;
            // Normalize axis value with asymmetric max points
            double normalize = qAbs(result / ((sign > 0) ? 32767.0 : -32768.0));
            double outputTemp = temp;
            outputTemp = (normalize * normalize);
            // Take original sign into account and compute output value
            result = outputTemp * (sign > 0 ? 32767 : -32768);

            break;
        }
        case AxisCurve::Cubic:
        {
            int sign = result > 0 ? 1 :-1;
            int temp = result;
            Q_UNUSED(sign);
            // Normalize axis value with asymmetric max points
            double normalize = qAbs(result / ((sign > 0) ? 32767.0 : -32768.0));
            double outputTemp = temp;

            outputTemp = normalize * normalize * normalize;

            // Take original sign into account and compute output value
            result = qAbs(outputTemp) * (sign > 0 ? 32767 : -32768);
            break;
        }
        case AxisCurve::Power:
        {
            int sign = result >= 0 ? 1 :-1;
            double difference = qAbs(result / ((result >= 0) ? static_cast<double>(XINPUT_AXIS_MAX) :
                                                               static_cast<double>(XINPUT_AXIS_MIN)));
            double sensitivity = axisSens[axis];
            double tempsensitive = qMin(qMax(sensitivity, 1.0e-3), 1.0e+3);
            double temp = qMin(qMax(pow(difference, 1.0 / tempsensitive), 0.0), 1.0);
            result = temp * (sign > 0 ? XINPUT_AXIS_MAX : XINPUT_AXIS_MIN);
            break;
        }
        case AxisCurve::Disabled:
        {
            // Always return axis as centered
            result = 0;
            break;
        }
    }

    return result;
}

AxisCurve::Type DS4DeviceTest::getCurveFromString(QString value)
{
    AxisCurve::Type result = AxisCurve::Linear;
    if (value == "linear")
    {
        result = AxisCurve::Linear;
    }
    else if (value == "enhanced-precision")
    {
        result = AxisCurve::EnhancedPrecision;
    }
    else if (value == "quadratic")
    {
        result = AxisCurve::Quadratic;
    }
    else if (value == "cubic")
    {
        result = AxisCurve::Cubic;
    }
    else if (value == "power")
    {
        result = AxisCurve::Power;
    }
    else if (value == "disabled")
    {
        result = AxisCurve::Disabled;
    }

    return result;
}

int DS4DeviceTest::calculateAxisValueAfterDead(int axis, int value)
{
    int result = value;
    int axisDeadZone = axisDeadZones[axis];
    int axisAntiDeadZone = axisAntiDeadZones[axis];

    // If no calculation needs to be done, save time and skip.
    if (axisDeadZone != 0 || axisAntiDeadZone != 0)
    {
        int maxDirValue = (value > 0) ? XINPUT_AXIS_MAX : XINPUT_AXIS_MIN;
        int currentDead = (axisDeadZone / 100.0) * maxDirValue;
        double currentAntiDead = (axisAntiDeadZone / 100.0);
        // Obtain normalized magnitude
        double tempValue = (qAbs(value) >= qAbs(currentDead)) ?
                    (value - currentDead) / static_cast<double>(maxDirValue - currentDead) :
                    0.0;

        if (tempValue > 0.0)
        {
            // Calculate anti-dead zone offset
            tempValue = (1.0 - currentAntiDead) * tempValue + currentAntiDead;
        }

        // Convert normalized value back to full range
        result = qFloor(tempValue * maxDirValue);
    }

    return result;
}

int DS4DeviceTest::calculateFlippedAxisValue(int value)
{
    int result = value;
    int maxNegDirValue = XINPUT_AXIS_MIN;
    int maxPosDirValue = XINPUT_AXIS_MAX;
    int maxDirValue = (value >= 0 ? (value / static_cast<double>(maxPosDirValue)) * maxNegDirValue : (value / static_cast<double>(maxNegDirValue)) * maxPosDirValue);
    result = maxDirValue;
    return result;
}

void DS4DeviceTest::changePollRate(int pollRate)
{
    if (pollRate >= 1 && pollRate <= 16)
    {
        testTimer.setInterval(pollRate);
        if (testTimer.isActive())
        {
            testTimer.start();
        }

        timeit.restart();
    }
}

void DS4DeviceTest::readControllerState()
{
    //byte inputReport[64] = {0};
    DWORD bytesRead = 0;
    //OVERLAPPED ol;
    //memset(&ol, 0, sizeof(ol));
    int numRead = 0;
    //bool result = ReadFile(m_fileHandle, &inputReport, 64, &bytesRead, 0);

    if (isWaitingOverLap)
    {
        if (GetOverlappedResult(m_fileHandle, &olu, &bytesRead, FALSE))
        {
            if (bytesRead == 64)
            {
                readInputReport();
                numRead++;
            }

            memset(&inputReport, 0, sizeof(inputReport));
            memset(&olu, 0, sizeof(olu));
            isWaitingOverLap = false;
        }
    }

    if (!isWaitingOverLap)
    {
        memset(&inputReport, 0, sizeof(inputReport));
        memset(&olu, 0, sizeof(olu));
        bool result = ReadFile(m_fileHandle, &inputReport, 64, 0, &olu);
        while (result)
        {
            numRead++;
            readInputReport();
            memset(&inputReport, 0, sizeof(inputReport));
            memset(&olu, 0, sizeof(olu));
            result = ReadFile(m_fileHandle, &inputReport, 64, 0, &olu);
        }

        if (!result && GetLastError() == ERROR_IO_PENDING)
        {
            isWaitingOverLap = true;
        }
        else
        {
            isWaitingOverLap = false;
            memset(&inputReport, 0, sizeof(inputReport));
            memset(&olu, 0, sizeof(olu));
        }
    }

    outputReport();
    //qDebug() << "ELAPSED: " << timeit.elapsed();
    //timeit.restart();

    //qDebug() << "RUNNING: " << bytesRead << " " << GetLastError();
}

void DS4DeviceTest::readInputReport()
{
    if (inputReport[0] == 1)
    {
        hat = 0;
        byte hatstate = (byte)(inputReport[5] & ((1 << 4) - 1));
        switch (hatstate)
        {
        case 0: hat = DS4_HAT_UP; break;
        case 1: hat = DS4_HAT_UP | DS4_HAT_RIGHT; break;
        case 2: hat = DS4_HAT_RIGHT; break;
        case 3: hat = DS4_HAT_RIGHT | DS4_HAT_DOWN; break;
        case 4: hat = DS4_HAT_DOWN; break;
        case 5: hat = DS4_HAT_LEFT | DS4_HAT_DOWN; break;
        case 6: hat = DS4_HAT_LEFT; break;
        case 7: hat = DS4_HAT_UP | DS4_HAT_LEFT; break;
        case 8:
        default: hat = DS4_HAT_CENTERED;
        }

        buttons[DS4_SQUARE_BUTTON] = (byte)(inputReport[5] & (1 << 4)) != 0;
        buttons[DS4_CROSS_BUTTON] = (byte)(inputReport[5] & (1 << 5)) != 0;
        buttons[DS4_CIRCLE_BUTTON] = (byte)(inputReport[5] & (1 << 6)) != 0;
        buttons[DS4_TRIANGLE_BUTTON] = (byte)(inputReport[5] & (1 << 7)) != 0;

        buttons[DS4_L1_BUTTON] = (byte)(inputReport[6] & (1 << 0)) != 0;
        buttons[DS4_R1_BUTTON] = (byte)(inputReport[6] & (1 << 1)) != 0;
        //buttons[DS4_L2_BUTTON] = (byte)(inputReport[6] & (1 << 1)) != 0;
        //buttons[DS4_R2_BUTTON] = (byte)(inputReport[6] & (1 << 1)) != 0;
        buttons[DS4_SHARE_BUTTON] = (byte)(inputReport[6] & (1 << 4)) != 0;
        buttons[DS4_OPTIONS_BUTTON] = (byte)(inputReport[6] & (1 << 5)) != 0;
        buttons[DS4_L3_BUTTON] = (byte)(inputReport[6] & (1 << 6)) != 0;
        buttons[DS4_R3_BUTTON] = (byte)(inputReport[6] & (1 << 7)) != 0;
        buttons[DS4_PS_BUTTON] = (byte)(inputReport[7] & (1 << 0)) != 0;
        buttons[DS4_TOUCHPAD_BUTTON] = (byte)(inputReport[7] & ((1 << 2) - 1)) != 0;

        axes[DS4_LX] = inputReport[1];
        axes[DS4_LY] = inputReport[2];
        axes[DS4_RX] = inputReport[3];
        axes[DS4_RY] = inputReport[4];
        axes[DS4_LTRIGGER] = inputReport[8];
        axes[DS4_RTRIGGER] = inputReport[9];
    }
}

void DS4DeviceTest::outputReport()
{
    byte *controllerreport = 0;
    //byte finalreport[28];
    //memset(&finalreport, 0, sizeof(finalreport));
    //busDevice->generateReport(controllerreport);
    generateXinputReport(controllerreport);

    byte outputReport[8];
    memset(&outputReport, 0, sizeof(outputReport));

    busDevice->report(static_cast<unsigned int>(xinputIndex), controllerreport, outputReport);

    if (controllerreport)
    {
        delete [] controllerreport;
        controllerreport = 0;
    }
}

void DS4DeviceTest::generateXinputReport(byte *&report)
{
    report = new byte[20];
    memset(report, 0x00, sizeof(*report) * 20);

    report[0] = 0x00;                                 // Message type (input report)
    report[1] = 0x14;                                 // Message size (20 bytes)

    unsigned int buttonsTest = 0;

    buttonsTest |= hat & DS4_HAT_UP ? DPAD_UP : 0;
    buttonsTest |= hat & DS4_HAT_RIGHT ? DPAD_RIGHT : 0;
    buttonsTest |= hat & DS4_HAT_DOWN ? DPAD_DOWN : 0;
    buttonsTest |= hat & DS4_HAT_LEFT ? DPAD_LEFT : 0;

    buttonsTest |= buttons[DS4_OPTIONS_BUTTON] ? START : 0;
    buttonsTest |= buttons[DS4_SHARE_BUTTON] ? BACK : 0;
    buttonsTest |= buttons[DS4_PS_BUTTON] ? GUIDE : 0;
    buttonsTest |= buttons[DS4_L3_BUTTON] ? LEFTSTICK : 0;
    buttonsTest |= buttons[DS4_R3_BUTTON] ? RIGHTSTICK : 0;
    buttonsTest |= buttons[DS4_L1_BUTTON] ? LEFTBUMPER : 0;
    buttonsTest |= buttons[DS4_R1_BUTTON] ? RIGHTBUMPER : 0;

    buttonsTest |= buttons[DS4_CROSS_BUTTON] ? A : 0;
    buttonsTest |= buttons[DS4_CIRCLE_BUTTON] ? B : 0;
    buttonsTest |= buttons[DS4_SQUARE_BUTTON] ? X : 0;
    buttonsTest |= buttons[DS4_TRIANGLE_BUTTON] ? Y : 0;

    report[2] = (byte)(buttonsTest >> 0 & 0xFF);  // Buttons low
    report[3] = (byte)(buttonsTest >> 8 & 0xFF);  // Buttons high

    //int triggerDeadZone = 25;
    //report[4] = (axes[DS4_LTRIGGER] > triggerDeadZone) ? static_cast<byte>((axes[DS4_LTRIGGER] - triggerDeadZone) / static_cast<double>(255 - triggerDeadZone) * 255) : 0;
    //report[5] = (axes[DS4_RTRIGGER] > triggerDeadZone) ? static_cast<byte>((axes[DS4_RTRIGGER] - triggerDeadZone) / static_cast<double>(255 - triggerDeadZone) * 255) : 0;
    report[4] = (axes[DS4_LTRIGGER] > axisDeadZones[DS4_LTRIGGER]) ? static_cast<byte>((axes[DS4_LTRIGGER] - axisDeadZones[DS4_LTRIGGER]) / static_cast<double>(255 - axisDeadZones[DS4_LTRIGGER]) * 255) : 0;
    report[5] = (axes[DS4_RTRIGGER] > axisDeadZones[DS4_RTRIGGER]) ? static_cast<byte>((axes[DS4_RTRIGGER] - axisDeadZones[DS4_RTRIGGER]) / static_cast<double>(255 - axisDeadZones[DS4_RTRIGGER]) * 255) : 0;

    int inputResolution = INPUT_HALF_AXIS_MAX - INPUT_HALF_AXIS_MIN;
    int outputResolution = XINPUT_AXIS_MAX - XINPUT_AXIS_MIN;

    int tempLx = computeSmoothedValue(DS4_LX);
    tempLx = qBound(INPUT_AXIS_MIN, qFloor(tempLx * axisScales[DS4_LX]), INPUT_AXIS_MAX);
    tempLx = (((tempLx - 0x80) - INPUT_HALF_AXIS_MIN) / static_cast<double>(inputResolution)) * outputResolution + XINPUT_AXIS_MIN;

    int tempLy = computeSmoothedValue(DS4_LY);
    tempLy = qBound(INPUT_AXIS_MIN, qFloor(tempLy * axisScales[DS4_LY]), INPUT_AXIS_MAX);
    double tempPosition = ((((tempLy - 0x80) - INPUT_HALF_AXIS_MIN) / static_cast<double>(inputResolution)) - 0.5) * -1 + 0.5;
    tempLy = tempPosition * outputResolution + XINPUT_AXIS_MIN;

    calculateStickValuesAfterDead(DS4_LX, DS4_LY, tempLx, tempLy, tempLx, tempLy);

    tempLx = getCurvedAxisValue(axisCurves[DS4_LX], tempLx, DS4_LX);

    short lxAxis = static_cast<short>(qBound(XINPUT_AXIS_MIN, tempLx, XINPUT_AXIS_MAX));
    report[6] = static_cast<byte>(lxAxis & 0xFF); // Left stick X-axis low
    report[7] = static_cast<byte>(lxAxis >> 8 & 0xFF); // Left stick X-axis high

    tempLy = getCurvedAxisValue(axisCurves[DS4_LY], tempLy, DS4_LY);
    short lyAxis = static_cast<short>(qBound(XINPUT_AXIS_MIN, tempLy, XINPUT_AXIS_MAX));
    report[8] = static_cast<byte>(lyAxis & 0xFF);           // Left stick Y-axis low
    report[9] = static_cast<byte>(lyAxis >> 8 & 0xFF);      // Left stick Y-axis high

    int tempRx = computeSmoothedValue(DS4_RX);
    tempRx = qBound(INPUT_AXIS_MIN, qFloor(tempRx * axisScales[DS4_RX]), INPUT_AXIS_MAX);
    tempRx = (((axes[DS4_RX] - 0x80) - INPUT_HALF_AXIS_MIN) / static_cast<double>(inputResolution)) *  outputResolution + XINPUT_AXIS_MIN;

    int tempRy = computeSmoothedValue(DS4_RY);
    tempRy = qBound(INPUT_AXIS_MIN, qFloor(tempRy * axisScales[DS4_RY]), INPUT_AXIS_MAX);
    double positionRy = ((((axes[DS4_RY] - 0x80) - INPUT_HALF_AXIS_MIN) / static_cast<double>(inputResolution)) - 0.5) * -1 + 0.5;
    tempRy = positionRy * outputResolution + XINPUT_AXIS_MIN;

    int tempOutRx = tempRx;
    int tempOutRy = tempRy;

    calculateStickValuesAfterDead(DS4_RX, DS4_RY, tempRx, tempRy, tempOutRx, tempOutRy);

    tempRx = tempOutRx;
    tempRy = tempOutRy;

    tempRx = getCurvedAxisValue(axisCurves[DS4_RX], tempRx, DS4_RX);
    short rxAxis = static_cast<short>(qBound(XINPUT_AXIS_MIN, tempRx, XINPUT_AXIS_MAX));
    report[10] = static_cast<byte>(rxAxis & 0xFF);          // Right stick X-axis low
    report[11] = static_cast<byte>(rxAxis >> 8 & 0xFF);     // Right stick X-axis high

    tempRy = getCurvedAxisValue(axisCurves[DS4_RY], tempRy, DS4_RY);
    short ryAxis = static_cast<short>(qBound(XINPUT_AXIS_MIN, tempRy, XINPUT_AXIS_MAX));
    report[12] = static_cast<byte>(ryAxis & 0xFF);          // Right stick Y-axis low
    report[13] = static_cast<byte>(ryAxis >> 8 & 0xFF);     // Right stick Y-axis high
}

void DS4DeviceTest::calculateStickValuesAfterDead(int axis1, int axis2, int axis1Value,
                                                  int axis2Value, int &outAxis1Value,
                                                  int &outAxis2Value)
{
    int axisXDeadTemp = axisDeadZones[axis1];
    int axisYDeadTemp = axisDeadZones[axis2];
    int axisXAntiDeadTemp = axisAntiDeadZones[axis1];
    int axisYAntiDeadTemp = axisAntiDeadZones[axis2];
    int axisXMaxTemp = axisMaxZones[axis1];
    int axisYMaxTemp = axisMaxZones[axis2];

    // If no calculation needs to be done, save time and skip.
    if (axisXDeadTemp != 0 || axisYDeadTemp != 0 ||
        axisXAntiDeadTemp != 0 || axisYAntiDeadTemp != 0 ||
        axisXMaxTemp != 100 || axisYMaxTemp != 100)
    {
        double angle = atan2(axis1Value, -axis2Value);
        double ang_sin = sin(angle);
        double ang_cos = cos(angle);

        int maxXDirValue = (axis1Value >= 0) ? XINPUT_AXIS_MAX : XINPUT_AXIS_MIN;
        int maxYDirValue = (axis2Value >= 0) ? XINPUT_AXIS_MAX : XINPUT_AXIS_MIN;

        int maxzoneXNegDirValue = (axisXMaxTemp / 100.0) * XINPUT_AXIS_MIN;
        int maxzoneXPosDirValue = (axisXMaxTemp / 100.0) * XINPUT_AXIS_MAX;
        int maxzoneYNegDirValue = (axisYMaxTemp / 100.0) * XINPUT_AXIS_MIN;
        int maxzoneYPosDirValue = (axisYMaxTemp / 100.0) * XINPUT_AXIS_MAX;

        int maxzoneXDirValue = (axis1Value >= 0 ? maxzoneXPosDirValue : maxzoneXNegDirValue);
        int maxzoneYDirValue = (axis2Value >= 0 ? maxzoneYPosDirValue : maxzoneYNegDirValue);

        int oldAxis1Value = axis1Value;

        axis1Value = qBound(maxzoneXNegDirValue, axis1Value, maxzoneXPosDirValue);
        axis2Value = qBound(maxzoneYNegDirValue, axis2Value, maxzoneYPosDirValue);

        int currentDeadX = qFloor((axisXDeadTemp / 100.0 * maxXDirValue) * qAbs(ang_sin));
        int currentDeadY = qFloor((axisYDeadTemp / 100.0 * maxYDirValue) * qAbs(ang_cos));

        double currentXAntiDead = (axisXAntiDeadTemp / 100.0) * qAbs(ang_sin);
        double currentYAntiDead = (axisYAntiDeadTemp / 100.0) * qAbs(ang_cos);

        double tempXValue = 0.0;
        double tempYValue = 0.0;

        //double tempXValueBefore = 0.0;
        //double tempYValueBefore = 0.0;

        int squareDist = (axis1Value * axis1Value) + (axis2Value * axis2Value);
        int deadDist = (currentDeadX * currentDeadX) + (currentDeadY * currentDeadY);

        if (squareDist > deadDist)
        {
            // Obtain normalized magnitude
            tempXValue = (axis1Value - currentDeadX) /
                    static_cast<double>(maxzoneXDirValue - currentDeadX);

            tempYValue = (axis2Value - currentDeadY) /
                    static_cast<double>(maxzoneYDirValue - currentDeadY);

            //tempXValueBefore = tempXValue;
            //tempYValueBefore = tempYValue;
        }

        if (tempXValue > 0.0)
        {
            // Calculate anti-dead zone offset
            tempXValue = (1.0 - currentXAntiDead) * tempXValue + currentXAntiDead;
        }

        if (tempYValue > 0.0)
        {
            // Calculate anti-dead zone offset
            tempYValue = (1.0 - currentYAntiDead) * tempYValue + currentYAntiDead;
        }

        // Convert normalized value back to full range
        outAxis1Value = qFloor(tempXValue * maxXDirValue);
        outAxis2Value = qFloor(tempYValue * maxYDirValue);

        if (axis1 == 2)
        {
            //qDebug() << "VALUES: " << oldAxis1Value << " | " << outAxis1Value << " | " << maxzoneXDirValue;
        }
    }
}

